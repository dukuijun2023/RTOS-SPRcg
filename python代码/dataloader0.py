from torch.utils.data import Dataset,DataLoader  
import os
import pandas as pd
import numpy as np 
import torch 
from scipy.io import wavfile
import time
import random
import math




class SpeechBlockManager:
    def __init__(self):
        """初始化块管理器（无初始数据）"""
        self.blocks = []              # 存储所有块：(block_id, label, audio_data)
        self.block_counter = 0        # 全局唯一的块ID计数器（从0开始递增）

    def add(self, audio_data, label):
        """
        添加单个带标签的1.5秒音频块
        
        参数:
            audio_data (np.ndarray): 单声道一维音频数据（1.5秒）
            label (str/int): 该块对应的人员标签（如"ID1"）
        
        异常:
            ValueError: 音频数据格式错误时抛出
        """
        # 分配ID并存储
        self.blocks.append((self.block_counter, label, audio_data))
        self.block_counter += 1

    def add_many(self, blocks, label):
        """
        批量添加多个同标签的1.5秒音频块（新增核心功能）
        
        参数:
            blocks (list): 同一人员的音频块列表（每个元素为np.ndarray，单声道一维）
            label (str/int): 这些块统一对应的标签（如"XiaoXin"）
        
        异常:
            ValueError: 块列表格式错误或音频数据格式错误时抛出
        """

        
        
        # 批量添加（复用单个add逻辑，确保ID递增）
        for audio in blocks:
            self.add(audio, label)

    def get_all_blocks(self):
        """返回所有块（格式：[(block_id, label, audio_data), ...]）"""
        return self.blocks.copy()

    def get_blocks_by_label(self, target_label):
        """按标签筛选块（返回：[(block_id, audio_data), ...]）"""
        return [(bid, label,data) for bid, label, data in self.blocks if label == target_label]

    def get_label_distribution(self):
        """统计各标签的块数量（返回：{label: count, ...}）"""
        distribution = {}
        for _, label, _ in self.blocks:
            distribution[label] = distribution.get(label, 0) + 1
        return distribution



class AudioProcessor:
    def __init__(self, sample_rate=8000, frame_length=0.05, hop_length=0.02):
        """
        初始化VAD处理器
        
        参数:
            sample_rate (int): 采样率（Hz），默认8000
            frame_length (float): 帧长（秒），默认0.025秒（25ms）
            hop_length (float): 帧移（秒），默认0.01秒（10ms）
        """
    def __init__(self, n_mfcc=13, sr=8000, frame_length_ms=50, frame_shift_ms=20, preemphasis_coef=0.97, n_fft=512):
        self.n_mfcc = n_mfcc          # 最终输出的MFCC系数数量（通常取13）
        self.sr = sr                  # 采样率（8000Hz）
        self.frame_length = int(sr * frame_length_ms / 1000)  # 帧长（50ms→400点）
        self.frame_shift = int(sr * frame_shift_ms / 1000)    # 帧移（20ms→160点）
        self.preemphasis_coef = preemphasis_coef  # 预加重系数（通常0.95~0.97）
        self.n_fft = n_fft            # FFT窗口大小（需≥帧长，常用512）
        self.n_mels = n_mfcc + 2      # 梅尔滤波器数量（比n_mfcc多2，用于边界）
        self.n_fft_bins = self.n_fft // 2 + 1  # FFT有效频率点数（实数信号对称，取前半）
        self.mel_filters = self._create_mel_filters()  # 预计算梅尔滤波器组                     # 帧移（秒）

    def _frame_audio(self, audio):
        """
        将音频分帧为固定长度的帧（内部辅助函数）
        
        参数:
            audio (np.ndarray): 原始音频数据（单声道）
            original_length (int): 原始音频长度（采样点）
            
        返回:
            frames (np.ndarray): 分帧后的音频（shape=(num_frames, frame_length)）
            frame_indices (list): 每帧在原始音频中的起始位置（采样点）
        """
        # 计算总帧数
        num_frames = (12000 - self.frame_length) // self.frame_shift
        frames = []
        audio = np.asarray(audio)  # 关键：转换为numpy数组
        for i in range(num_frames):
            frame = audio[i * self.frame_shift:i * self.frame_shift + self.frame_length]
            frames.append(frame)
        return frames

    def _cal_per_energy(self, frames):
        """
        计算每帧的短时能量与RMS（内部辅助函数）
        
        参数:
            frames (np.ndarray): 分帧后的音频（shape=(num_frames, frame_length)）
            
        返回:
            energy (np.ndarray): 每帧的短时能量
            rms_list (np.ndarray): 每帧的RMS（dB）
        """
        energys = []
        for frame in frames:
            p_ref = 20e-6  # 20微帕
            max_quant = 2 ** (16 - 1)
            p_max = p_ref * max_quant  # 满量程声压（Pa）

            frame_normalized = frame.astype(np.float32) / max_quant
          
            
            # 转换为声压值（Pa）
            p_samples = frame_normalized * p_max
            
            # ---------------------- 步骤3：计算有效声压（RMS） ----------------------
            p_rms = np.sqrt(np.mean(p_samples ** 2))
            
            # ---------------------- 步骤4：计算分贝值 ----------------------
            if p_rms == 0:
                # 避免对数运算错误（静音返回-∞，实际中可设为极小值）
                p_rms = p_rms + 1
            db = 20 * np.log10(p_rms / p_ref)
            energys.append(db)
        return energys

    
    def remove_zero_speech_rows(self,output_array):
        """
        删除输出数组中最后一列（is_speech）为0的行，仅保留is_speech=1的行
        
        参数:
            output_array (np.ndarray): 原始输出数组（形状为 [N, 4]，最后一列为 is_speech）
        
        返回:
            np.ndarray: 过滤后的输出数组（仅保留is_speech=1的行）
        """
        # 若输入数组为空，直接返回空数组
        if output_array.size == 0:
            return np.empty((0, 4), dtype=int)
        
        # 创建布尔掩码：最后一列（is_speech）为1时保留（True）
        mask = output_array['is_speech'] == 1  # 第4列（索引3）为1时，mask对应位置为True
        
        # 筛选符合条件的行（仅保留mask为True的行）
        filtered_array = output_array[mask]
        
        return filtered_array

    def _preemphasis(self, signal):
        """预加重：提升高频成分"""
        return np.append(signal[0], signal[1:] - self.preemphasis_coef * signal[:-1])

    def _hamming_window(self, frame_length):
        """生成汉明窗"""
        return 0.54 - 0.46 * np.cos(2 * np.pi * np.arange(frame_length) / (frame_length - 1))

    def _create_mel_filters(self):
        """创建梅尔滤波器组（三角滤波器）"""
        # 1. 计算梅尔频率范围（0到奈奎斯特频率的梅尔值）
        max_freq = self.sr // 2  # 奈奎斯特频率（8000/2=4000Hz）
        min_mel = 0
        max_mel = 2595 * np.log10(1 + max_freq / 700)  # 梅尔频率转换公式
        
        # 2. 生成n_mels个均匀分布的梅尔中心频率（线性频率）
        mel_centers = np.linspace(min_mel, max_mel, self.n_mels)  # 长度为n_mels
        hz_centers = 700 * (10 ** (mel_centers / 2595) - 1)  # 转换为Hz（长度为n_mels）
        
        # 3. 生成三角滤波器的边界（Hz），数量为n_mels + 1
        hz_edges = np.zeros(self.n_mels + 1)
        hz_edges[1:-1] = (hz_centers[:-1] + hz_centers[1:]) / 2  # 相邻中心的中点
        
        # 4. 将频率边界转换为FFT的bin索引（关键修复：从Hz到索引）
        # FFT的有效频率范围是0到sr/2，对应bin索引0到n_fft_bins-1
        n_fft_bins = self.n_fft // 2 + 1
        bin_edges = (hz_edges * self.n_fft) // self.sr  # 频率→bin索引（整数）
        bin_centers = (hz_centers * self.n_fft) // self.sr  # 中心频率→bin索引（整数）
        
        # 5. 构建梅尔滤波器组（n_mels × n_fft_bins）
        filters = np.zeros((self.n_mels, n_fft_bins))
        for i in range(self.n_mels):
            # 当前滤波器的左右边界（bin索引）
            left = int(round(bin_edges[i]))  # 左边界索引（整数）
            right = int(round(bin_edges[i+1]))  # 右边界索引（整数）
            center = int(round(bin_centers[i]))  # 中心索引（整数）
            
            # 限制边界在有效范围内（避免越界）
            left = max(0, left)
            right = min(n_fft_bins - 1, right)
            center = max(left, min(center, right))  # 中心必须在[left, right]内
            
            # 左半部分：从左边界到中心（斜率递增）
            if left < center:
                slope = 1.0 / (center - left)
                for j in range(left, center):
                    filters[i, j] = slope * (j - left)
            
            # 右半部分：从中心到右边界（斜率递减）
            if right > center:
                slope = 1.0 / (right - center)
                for j in range(center, right):
                    filters[i, j] = slope * (right - j)
    
        return filters

    def _apply_mel_filters(self, magnitude_spectrum):
        """应用梅尔滤波器组，计算每个滤波器的能量和"""
        # 确保幅度谱长度与滤波器索引匹配（补零到n_fft_bins）
        if len(magnitude_spectrum) < self.n_fft_bins:
            magnitude_spectrum = np.pad(magnitude_spectrum, (0, self.n_fft_bins - len(magnitude_spectrum)), mode='constant')
        
        # 每个滤波器的能量和 = 滤波器系数 × 幅度谱的平方（能量）
        mel_energies = np.dot(self.mel_filters, magnitude_spectrum ** 2)
        return mel_energies

    def _dct(self, log_mel_energies):
        """离散余弦变换（DCT）得到MFCC系数"""
        # 使用Type-II DCT（常用MFCC的DCT类型）
        n = len(log_mel_energies)
        dct_coeffs = np.zeros(n)
        for k in range(n):
            sum_val = 0.0
            for m in range(n):
                sum_val += log_mel_energies[m] * np.cos(np.pi * k * (m + 0.5) / n)
            dct_coeffs[k] = sum_val * np.sqrt(2 / n)
        return dct_coeffs



    def process(self, input_array):
        """
        处理输入的语音块数组，输出语音帧数组
        
        参数:
            input_array (np.ndarray): 输入数组（shape=(90, 3)），每行格式为：
                [语音文件路径（str）, 语音顺序编号（int）, 人员编号（int）]
                
        返回:
            output_array (np.ndarray): 输出数组（shape=(N, 4)），每行格式为：
                [原语音顺序编号（int）, 人员编号（int）, 帧顺序编号（int）, 是否语音（int）]
                （仅保留语音帧）
        """
        output_frames = []  # 存储最终输出的语音帧信息

        
        # 遍历每个输入的语音块
        for block in input_array:
            block_order,person_ids ,audio_data = block  # 解包块信息
            
            # 1. 分帧（生成帧数据和元信息）
            frames = self._frame_audio(audio_data)
            num_frames = len(frames)
            # 2. 计算帧特征（能量）
            energys= self._cal_per_energy(frames)

            # 3. 生成帧顺序编号（全局唯一）
            frame_order = [block_order * 100 + i for i in range(num_frames)]  # 示例编号规则

            # 4. 对每帧自主实现MFCC计算
            frame_mfcc_list = []
            for frame in frames:
                # 步骤1：预加重（提升高频）
                preemphasized = self._preemphasis(frame)
                
                # 步骤2：加汉明窗（减少频谱泄漏）
                windowed = preemphasized * self._hamming_window(self.frame_length)
                
                # 步骤3：FFT并取幅度谱（仅保留正频率部分）
                fft_spectrum = np.fft.rfft(windowed, n=self.n_fft)
                magnitude_spectrum = np.abs(fft_spectrum)  # 幅度谱（形状=(n_fft_bins,)）
                
                # 步骤4：应用梅尔滤波器组（计算每个滤波器的能量和）
                mel_energies = self._apply_mel_filters(magnitude_spectrum)  # 形状=(n_mels,)
                
                # 步骤5：对数能量（避免零值，加小量1e-6）
                log_mel_energies = np.log(mel_energies + 1e-6)
                
                # 步骤6：DCT（离散余弦变换）得到MFCC系数
                mfcc = self._dct(log_mel_energies)  # 形状=(n_mels,)
                
                # 取前n_mfcc个系数（通常13维）
                frame_mfcc = mfcc[:self.n_mfcc]
                frame_mfcc_list.append(frame_mfcc)
            
            # 7. 合并元信息（原块顺序、人员编号、帧顺序）
            for i in range(num_frames):
                frame_info = {
                    'block_order': block_order,
                    'person_id': person_ids,
                    'frame_order': frame_order[i],
                    'is_speech': 0 , # 初始化为非语音
                    'mfcc': frame_mfcc_list[i]     # 自主实现的MFCC列表
                }
                
                # 8. 应用VAD逻辑（标记语音帧）
                # 条件1：能量高于平均能量
                ave_energy = 40
                is_high_energy = energys[i] >= ave_energy         
                if is_high_energy:
                    frame_info['is_speech'] = 1

                output_frames.append(frame_info)
        
        # 9. 转换为输出数组（按原顺序排列，仅保留语音帧）
        if not output_frames:
            return np.empty((0, 5), dtype=[('block_order', int), ('person_id', int), 
                                          ('frame_order', int), ('is_speech', int), ('mfcc', 'O')])
    
        # 按原块顺序和帧顺序排序（确保顺序正确）
        output_frames.sort(key=lambda x: (x['block_order'], x['frame_order']))
        # 提取需要的列（原块顺序、人员编号、帧顺序、是否语音）
        output_array = np.array([
        (
            f['block_order'], 
            f['person_id'], 
            f['frame_order'], 
            f['is_speech'],
            np.array(f['mfcc'], dtype=np.float32)  # 转换为13维浮点数数组
        ) 
        for f in output_frames
    ], dtype=[
        ('block_order', int), 
        ('person_id', int), 
        ('frame_order', int), 
        ('is_speech', int), 
        ('mfcc', float, (13,))  # 固定MFCC维度为13
    ])
         
        output_array = self.remove_zero_speech_rows(output_array)
        return output_array




   