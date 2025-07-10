import os
import numpy as np 
from scipy.io import wavfile
from dataloader0 import SpeechBlockManager,AudioProcessor
from sklearn.mixture import GaussianMixture
import joblib
from pathlib import Path


# 创建模型保存目录
model_dir = Path(r"D:\my_vs_code\per\VeriHealthi_QEMU_SDK.202505_preliminary\VeriHealthi_QEMU_SDK.202505_preliminary\VerHealthi_preliminary\models")
model_dir.mkdir(parents=True, exist_ok=True)

def split_wav_into_1_5s(folder_path):
        """
        将指定文件夹下的所有WAV文件分割为1.5秒大小的纯数据块，返回数据块数组及总数量
        
        参数:
            folder_path (str): 包含WAV文件的文件夹路径
        
        返回:
            tuple: (blocks_array, total_blocks)
                blocks_array (np.ndarray): 所有1.5秒数据块的数组（元素为np.ndarray类型）
                total_blocks (int): 数据块的总数量
        """
        # 筛选文件夹下所有WAV文件（不区分大小写）
        wav_files = [f for f in os.listdir(folder_path) 
                    if f.lower().endswith('.wav')]
        
        all_blocks = []
        
        for file in wav_files:
            file_path = os.path.join(folder_path, file)
            
            try:
                # 读取WAV文件元数据和音频数据
                sample_rate,audio_data = wavfile.read(file_path)
            except Exception as e:
                print(f"警告：跳过无法读取的文件 {file}，错误：{str(e)}")
                continue
            
            samples_per_block = int(1.5 * sample_rate)
            total_samples = audio_data.shape[0]
            num_blocks = total_samples  // samples_per_block
            for i in range(num_blocks):
                start = i * samples_per_block
                end = start + samples_per_block
                block = audio_data[start:end]
                all_blocks.append(block)
            
        # 转换为numpy数组（兼容不同形状的块，结果为object类型数组）
        blocks_array = np.array(all_blocks) if all_blocks else np.array([])
        return blocks_array.tolist()



def split_bin_into_1_5s(folder_path):
    """
    将指定文件夹下的所有BIN音频文件分割为1.5秒大小的纯数据块
    
    参数:
        folder_path (str): 包含BIN音频文件的文件夹路径
        
    返回:
        tuple: (blocks_list, total_blocks)
            blocks_list (list): 所有1.5秒数据块的列表（元素为np.ndarray类型）
            total_blocks (int): 数据块的总数量
    """
    # 筛选文件夹下所有BIN文件
    bin_files = [f for f in os.listdir(folder_path) 
                if f.lower().endswith('.bin')]
    
    blocks_list = []
    sample_rate = 8000  # 固定采样率8000 Hz
    samples_per_block = int(1.5 * sample_rate)  # 1.5秒的样本数
    
    for file in bin_files:
        file_path = os.path.join(folder_path, file)
        
        try:
            # 读取BIN文件的原始二进制数据
            with open(file_path, 'rb') as bin_file:
                raw_data = bin_file.read()
            
            # 将二进制数据转换为int16类型的NumPy数组
            # 假设BIN文件存储的是16位PCM数据（每个样本2字节）
            audio_data = np.frombuffer(raw_data, dtype=np.int16)
            
            total_samples = len(audio_data)
            num_blocks = total_samples // samples_per_block
            
            # 分割音频块
            for i in range(num_blocks):
                start = i * samples_per_block
                end = start + samples_per_block
                block = audio_data[start:end]
                blocks_list.append(block)
                
        except Exception as e:
            print(f"警告：跳过无法处理的文件 {file}，错误：{str(e)}")
            continue
            
    total_blocks = len(blocks_list)
    return blocks_list, total_blocks

def split_train_and_test(mfcc_datas,labels):
    train_size = int(len(labels) * 0.9)
    train_dataset = {'mfcc':mfcc_datas[:train_size],'labels':labels[:train_size]}
    test_dataset = {'mfcc':mfcc_datas[train_size+1:],'labels':labels[train_size+1:]}
    return train_dataset,test_dataset 


def train_speaker_models(speaker_data, n_components=64):
    models = {}
    
    for speaker_id, data in speaker_data.items():
        X = data["mfcc"]
        # 使用监督信息加权训练（可选）
        labels = data["labels"]
        
        # 初始化GMM模型
        gmm = GaussianMixture(
            n_components=n_components,
            covariance_type='diag',  # 推荐对角协方差
            reg_covar=1e-6,          # 正则化防止奇异矩阵
            random_state=42
        )
        
        # 训练模型
        gmm.fit(X,labels)
        
        # 保存模型参数
        models[speaker_id] = {
            'means': gmm.means_,
            'covariances': gmm.covariances_,
            'weights': gmm.weights_
        }
    
    return models

ID1datapath = ".\speakers_pcphone\ID1"
XiaoYuandatapath = ".\speakers_pcphone\XiaoYuan"
XiaoXindatapath = ".\speakers_pcphone\XiaoXin"
ID2datapath = ".\speakers_pcphone\ID2"
ID3datapath = ".\speakers_pcphone\ID3"
ID4datapath = ".\speakers_pcphone\ID4"
ID5datapath = ".\speakers_pcphone\ID5"
ID1datapath_bin = ".\speakers_board\ID1"
ID2datapath_bin = ".\speakers_board\ID2"
ID3datapath_bin = ".\speakers_board\ID3"
ID4datapath_bin = ".\speakers_board\ID4"
ID5datapath_bin = ".\speakers_board\ID5"
XiaoXindatapath_bin = ".\speakers_board\XiaoXin"
XiaoYuandatapath_bin = ".\speakers_board\XiaoYuan"


if __name__ == "__main__":
    # 模拟人员ID列表（实际从文件名或元数据获取）
    person_ids = ["ID1", "XiaoXin", "XiaoYuan"]

    block_ID1 = split_wav_into_1_5s(ID1datapath)
    block_ID2 = split_wav_into_1_5s(ID2datapath)
    block_ID3 = split_wav_into_1_5s(ID3datapath)
    block_ID4 = split_wav_into_1_5s(ID4datapath)
    block_ID5 = split_wav_into_1_5s(ID5datapath)
    block_xiaoXin = split_wav_into_1_5s(XiaoXindatapath)
    block_xiaoYuan = split_wav_into_1_5s(XiaoYuandatapath)

    block_ID1_bin = split_wav_into_1_5s(ID1datapath_bin)
    block_ID2_bin = split_wav_into_1_5s(ID2datapath_bin)
    block_ID3_bin = split_wav_into_1_5s(ID3datapath_bin)
    block_ID4_bin = split_wav_into_1_5s(ID4datapath_bin)
    block_ID5_bin = split_wav_into_1_5s(ID5datapath_bin)
    block_xiaoXin_bin = split_wav_into_1_5s(XiaoXindatapath_bin)
    block_xiaoYuan_bin = split_wav_into_1_5s(XiaoYuandatapath_bin)

    
    block_ID1 = np.array(block_ID1) if block_ID1 else np.array([])
    block_ID2 = np.array(block_ID2) if block_ID2 else np.array([])
    block_ID3 = np.array(block_ID3) if block_ID3 else np.array([])
    block_ID4 = np.array(block_ID4) if block_ID4 else np.array([])
    block_ID5 = np.array(block_ID5) if block_ID5 else np.array([])
    block_xiaoXin = np.array(block_xiaoXin) if block_xiaoXin else np.array([])
    block_xiaoYuan = np.array(block_xiaoYuan) if block_xiaoYuan else np.array([])

    block_ID1_bin = np.array(block_ID1_bin) if block_ID1_bin else np.array([])
    block_ID2_bin = np.array(block_ID2_bin) if block_ID2_bin else np.array([])
    block_ID3_bin = np.array(block_ID3_bin) if block_ID3_bin else np.array([])
    block_ID4_bin = np.array(block_ID4_bin) if block_ID4_bin else np.array([])
    block_ID5_bin = np.array(block_ID5_bin) if block_ID5_bin else np.array([])
    block_xiaoXin_bin = np.array(block_xiaoXin_bin) if block_xiaoXin_bin else np.array([])
    block_xiaoYuan_bin = np.array(block_xiaoYuan_bin) if block_xiaoYuan_bin else np.array([])

    
    # 初始化块管理器
    block_manager = SpeechBlockManager()
    block_manager.add_many(block_xiaoXin,1)
    block_manager.add_many(block_xiaoYuan,2)
    block_manager.add_many(block_ID1,0)
    block_manager.add_many(block_ID2,0)
    block_manager.add_many(block_ID3,0)
    block_manager.add_many(block_ID4,0)
    block_manager.add_many(block_ID5,0)
    
    block_manager.add_many(block_xiaoXin_bin,1)
    block_manager.add_many(block_xiaoYuan_bin,2)
    block_manager.add_many(block_ID1_bin,0)
    block_manager.add_many(block_ID2_bin,0)
    block_manager.add_many(block_ID3_bin,0)
    block_manager.add_many(block_ID4_bin,0)
    block_manager.add_many(block_ID5_bin,0)


    blocks_train_xiaoXin = block_manager.get_blocks_by_label(1)
    blocks_train_xiaoYuan = block_manager.get_blocks_by_label(2)

    
    audio_processor = AudioProcessor(
    n_mfcc=13,sr=8000
)
    #给所有识别人训练一个基准gmm，来判断输入是否为七人中的一个
    blocks_ubm_data = block_manager.get_all_blocks()
    blocks_ubm_data = audio_processor.process(blocks_ubm_data)
    all_datas = blocks_ubm_data['mfcc']
    ubm = GaussianMixture(n_components=64,covariance_type='diag',max_iter=100)
    ubm.fit(all_datas)


    output_array_xiaoXin = audio_processor.process(blocks_train_xiaoXin)
    X_xiaoXin = output_array_xiaoXin['mfcc']
    y_xiaoXin = output_array_xiaoXin['person_id']
    gmm_xiaoXin = GaussianMixture(n_components=64, covariance_type='diag', max_iter=100)
    gmm_xiaoXin.weights_init = ubm.weights_  # 用UBM初始化
    gmm_xiaoXin.means_init = ubm.means_
    gmm_xiaoXin.fit(X_xiaoXin)

    output_array_xiaoYuan = audio_processor.process(blocks_train_xiaoYuan)
    X_xiaoYuan = output_array_xiaoYuan['mfcc']
    y_xiaoYuan = output_array_xiaoYuan['person_id']
    gmm_xiaoYuan = GaussianMixture(n_components=64, covariance_type='diag', max_iter=100)
    gmm_xiaoYuan.weights_init = ubm.weights_  # 用UBM初始化
    gmm_xiaoYuan.means_init = ubm.means_
    gmm_xiaoYuan.fit(X_xiaoYuan)

    target_speakers = ["xiaoXin", "xiaoYuan"]

    ubm = joblib.load(model_dir / "ubm_model.pkl")
    np.savez(model_dir / "ubm_params.npz", 
            weights=ubm.weights_,
            means=ubm.means_,
            covariances=ubm.covariances_)

    for speaker in target_speakers:
        model = joblib.load(model_dir / f"gmm_{speaker}.pkl")
        np.savez(model_dir / f"gmm_{speaker}_params.npz",
                weights=model.weights_,
                means=model.means_,
                covariances=model.covariances_)

    def convert_to_bin(npz_path, bin_path):
        data = np.load(npz_path)
        with open(bin_path, 'wb') as f:
            # 写入权重
            data['weights'].astype('float32').tofile(f)
            # 写入均值
            for row in data['means']:
                row.astype('float32').tofile(f)
            # 写入协方差
            for row in data['covariances']:
                row.astype('float32').tofile(f)

    # 转换所有模型
    convert_to_bin('models/ubm_params.npz', 'models/ubm_params.bin')
    convert_to_bin('models/gmm_xiaoXin_params.npz', 'models/gmm_xiaoXin_params.bin')
    convert_to_bin('models/gmm_xiaoYuan_params.npz', 'models/gmm_xiaoYuan_params.bin')
    