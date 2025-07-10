#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "osal_heap_api.h"


// 启用RISC-V P扩展指令加速乘法
/*
__attribute__((optimize("target-arch=rv32imap")))
void q15_multiply(int16_t *a, int16_t *b, int16_t *c, int len) {
    for (int i = 0; i < len; i++) {
        asm volatile ("mul %0, %1, %2" : "=r"(c[i]) : "r"(a[i]), "r"(b[i]));
    }
}
*/

#define SAMPLE_RATE_HZ     8000
#define BLOCK_DURATION_S   1.5f
//#define SAMPLES_PER_BLOCK  (uint16_t)(SAMPLE_RATE_HZ * BLOCK_DURATION_S) // 12,000
#define TOTAL_BLOCKS       4 // 48,000 / 12,000
// 计算公式：帧数 = (总样本数 - 帧长) / 帧移 + 1
#define SAMPLES_PER_BLOCK 12000  // 1.5s * 8000Hz
#define FRAME_LEN 400            // 50ms帧长
#define FRAME_SHIFT 160          // 20ms帧移
#define FRAMES_PER_BLOCK ((SAMPLES_PER_BLOCK - FRAME_LEN) / FRAME_SHIFT )  // =72帧
#define N_COMPONENTS 64
#define N_FEATURES 13       // MFCC特征维度
#define PI 3.14159265358979323846
#define MEL_FILTERS 40     // 梅尔滤波器数量
#define FFT_LEN 400


typedef struct {
    uint16_t frame_data[FRAME_LEN];  // 帧音频数据（400字节）
    int start_time;               // 帧起始时间（秒）
    int energy;                    //分贝
    uint16_t mfcc[N_FEATURES];
    int is_speech;
    //int result;
} AudioFrame;

// 分块存储结构：每个块包含72帧
typedef struct {
    AudioFrame frames[FRAMES_PER_BLOCK];
} BlockFrames;

// 全局存储（4个块）
BlockFrames processed_blocks[TOTAL_BLOCKS];  // TOTAL_BLOCKS=4
// 分块结果：指针数组（每个指针指向一个1.5秒块的起始地址）
uint16_t* audio_blocks[TOTAL_BLOCKS];

void compute_mfcc(uint16_t* input_frame, uint16_t* mfcc_out, float sample_rate);
void dataloader(uint16_t* raw_data);
void split_audio_into_blocks (uint16_t* raw_data) {
    for (int block_idx = 0; block_idx < TOTAL_BLOCKS; block_idx++) {
        // 计算当前块的起始指针位置（零拷贝）
        uint16_t* block_start = (uint16_t*)(raw_data + (block_idx * SAMPLES_PER_BLOCK));
        audio_blocks[block_idx] = block_start;
    }
}

void split_into_frames(uint16_t** audio_blocks) {
    int current_time = 0;  // 起始时间从0开始

    for (int block_idx = 0; block_idx < TOTAL_BLOCKS; block_idx++) {
        uint16_t* block_data = audio_blocks[block_idx];

        for (int frame_idx = 0; frame_idx < FRAMES_PER_BLOCK; frame_idx++) {
            int offset = frame_idx * FRAME_SHIFT;

            memcpy(
                processed_blocks[block_idx].frames[frame_idx].frame_data,
                block_data + offset,
                FRAME_LEN
            );

            processed_blocks[block_idx].frames[frame_idx].start_time = current_time;
            current_time += 20;  // 20ms帧移
        }
    }
}

//实现能量阈值法：将帧数据转化为分贝，小于四十的删除，保存原顺序


int calculate_frame_energy(uint16_t* frame, int frame_length) {
    const double p_ref = 20e-6;     // 参考声压 (20微帕)
    const double max_quant = 32768.0; // 16位量化最大值 (2^(16-1))

    // 1. 计算满量程声压
    const double p_max = p_ref * max_quant;

    // 2. 计算平方和（用于RMS）
    double square_sum = 0.0;
    for (int i = 0; i < frame_length; i++) {
        // 归一化采样值
        double normalized = (double)frame[i] / max_quant;
        // 转换为声压值(Pa)
        double p_sample = normalized * p_max;
        // 累加平方值
        square_sum += p_sample * p_sample;
    }

    // 3. 计算RMS声压
    double p_rms = sqrt(square_sum / frame_length);

    // 4. 避免对数计算错误（静音帧处理）
    if (p_rms < 1e-20) p_rms = 1e-20;  // 安全阈值[8](@ref)

    // 5. 计算分贝值
    int tmp = 20 * (log10(p_rms / p_ref));
    return tmp;
}

// 主函数：计算所有帧的分贝值
void cal_per_energy(uint16_t** audio_blocks, int num_frames, int frame_length) {
    for (int block_idx = 0; block_idx < TOTAL_BLOCKS; block_idx++){
        // 逐帧处理
        for (int i = 0; i < num_frames; i++) {
            processed_blocks[block_idx].frames[i].energy = (int)calculate_frame_energy(processed_blocks[block_idx].frames[i].frame_data, frame_length);
            if(processed_blocks[block_idx].frames[i].energy <= 40){
                processed_blocks[block_idx].frames[i].is_speech = 0;
            }else processed_blocks[block_idx].frames[i].is_speech = 1;
        }
}
}




typedef struct {
    float weights[N_COMPONENTS];
    float means[N_COMPONENTS][N_FEATURES];
    float covariances[N_COMPONENTS][N_FEATURES];
} GMM_Model;

GMM_Model ubm;                 // UBM模型
GMM_Model gmm_xiaoXin;         // 小新模型
GMM_Model gmm_xiaoYuan;        // 小园模型

void load_gmm_params(const char* path, GMM_Model* model) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Error opening: %s\n", path);
        //exit(1);
    }

    fread(model->weights, sizeof(float), N_COMPONENTS, file);
    for (int i = 0; i < N_COMPONENTS; i++) {
        fread(model->means[i], sizeof(float), N_FEATURES, file);
    }
    for (int i = 0; i < N_COMPONENTS; i++) {
        fread(model->covariances[i], sizeof(float), N_FEATURES, file);
    }

    fclose(file);
}

void init_models() {
    load_gmm_params("models/ubm_params.bin", &ubm);
    load_gmm_params("models/gmm_xiaoXin_params.bin", &gmm_xiaoXin);
    load_gmm_params("models/gmm_xiaoYuan_params.bin", &gmm_xiaoYuan);
}

// 计算单高斯分量的对数概率密度
float gaussian_log_prob(float* x, float* mean, float* covar, int dim) {
    float log_prob = 0.0;
    for (int d = 0; d < dim; d++) {
        float diff = x[d] - mean[d];
        log_prob -= 0.5 * (diff * diff / covar[d] + logf(covar[d])); // 使用logf处理float
    }
    return log_prob;
}

// 计算GMM的对数似然
int gmm_log_prob(GMM_Model* model, uint16_t* xx) {
    float log_prob = -1e38; // 初始化为很小的数（log(0)=-inf）
    float* x = (float*)xx;
    for (int k = 0; k < N_COMPONENTS; k++) {
        float comp_log_prob = logf(model->weights[k]) + // 使用logf处理float
                             gaussian_log_prob(x, model->means[k], model->covariances[k], N_FEATURES);

        // LogSumExp算法（避免数值溢出）
        if (comp_log_prob > log_prob) {
            log_prob = comp_log_prob;
        }
    }
    int result = (int)log_prob;
    return result;
}

// 计算对数似然比
void compute_llr(uint16_t** audio_blocks, int num_frames, int frame_length,uint8_t * result) {
    float sum_llr_xin = 0.0;
    float sum_llr_yuan = 0.0;
    for (int block_idx = 0; block_idx < TOTAL_BLOCKS; block_idx++){
        for (int i = 0; i < num_frames; i++) {
            if(processed_blocks[block_idx].frames->is_speech == 0){
                continue;
            }
            uint16_t* frame = (uint16_t*)(processed_blocks[block_idx].frames->mfcc);
            int logp_ubm = gmm_log_prob(&ubm, frame);
            int logp_xin = gmm_log_prob(&gmm_xiaoXin, frame);
            int logp_yuan = gmm_log_prob(&gmm_xiaoYuan, frame);
            sum_llr_xin += (logp_xin - logp_ubm);
            sum_llr_yuan += (logp_yuan - logp_ubm);
            if(sum_llr_xin / num_frames < 0.5 && sum_llr_yuan / num_frames < 0.5) result[block_idx] = 0;
            result[i] = (sum_llr_xin / num_frames) > (sum_llr_yuan / num_frames)? 1 : 2;
        }
    }
}

//实现将帧数据转化为mfcc特征
void frames_to_mfcc(uint16_t** audio_blocks, int num_frames, int frame_length){
    for (int block_idx = 0; block_idx < TOTAL_BLOCKS; block_idx++){
        // 逐帧处理
        for (int i = 0; i < num_frames; i++) {
            for(int j = 0;j < 400;++j){
                processed_blocks[block_idx].frames[i].frame_data[j] = (short)(3000 * sin(2 * PI * 1000 * j / 8000) +
                               2000 * sin(2 * PI * 3000 * j / 8000));
            }
            float sample = 8000.0;
            compute_mfcc(processed_blocks[block_idx].frames[i].frame_data,processed_blocks[block_idx].frames[i].mfcc,sample);
        }
    }
}


// 预加重系数
static const float PREEMPHASIS_ALPHA = 0.97f;
// 汉明窗系数（400点）
static float HAMMING_WINDOW[FRAME_LEN];

// 初始化汉明窗（只需调用一次）
void init_hamming_window() {
    for (int i = 0; i < FRAME_LEN; i++) {
        HAMMING_WINDOW[i] = 0.54f - 0.46f * cos(2 * PI * i / (FRAME_LEN - 1));
    }
}

// 快速傅里叶变换（基2算法）
void fft(float* x_real, float* x_imag, int n) {
    int i, j, k, m;
    float t_real, t_imag, u_real, u_imag, w_real, w_imag;

    // 位反转重排
    j = 0;
    for (i = 0; i < n - 1; i++) {
        if (i < j) {
            t_real = x_real[i];
            t_imag = x_imag[i];
            x_real[i] = x_real[j];
            x_imag[i] = x_imag[j];
            x_real[j] = t_real;
            x_imag[j] = t_imag;
        }
        k = n / 2;
        while (k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }

    // 蝶形运算
    for (m = 2; m <= n; m *= 2) {
        float angle = -2 * PI / m;
        w_real = cos(angle);
        w_imag = sin(angle);

        for (k = 0; k < n; k += m) {
            u_real = 1.0f;
            u_imag = 0.0f;

            for (j = 0; j < m / 2; j++) {
                int idx1 = k + j;
                int idx2 = idx1 + m / 2;

                // 蝶形计算
                t_real = x_real[idx2] * u_real - x_imag[idx2] * u_imag;
                t_imag = x_real[idx2] * u_imag + x_imag[idx2] * u_real;

                x_real[idx2] = x_real[idx1] - t_real;
                x_imag[idx2] = x_imag[idx1] - t_imag;
                x_real[idx1] = x_real[idx1] + t_real;
                x_imag[idx1] = x_imag[idx1] + t_imag;

                // 更新旋转因子
                float next_u_real = u_real * w_real - u_imag * w_imag;
                u_imag = u_real * w_imag + u_imag * w_real;
                u_real = next_u_real;
            }
        }
    }
}

// 创建梅尔滤波器组
void create_mel_filters(float sample_rate, float** filters) {
    const float min_mel = 0.0f;
    const float max_mel = 2595.0f * log10f(1.0f + (sample_rate / 2) / 700.0f);

    // 在梅尔刻度上均匀分布的频率点
    float mel_points[MEL_FILTERS + 2];
    for (int i = 0; i < MEL_FILTERS + 2; i++) {
        mel_points[i] = min_mel + (max_mel - min_mel) * i / (MEL_FILTERS + 1);
    }

    // 将梅尔频率转换为Hz
    float hz_points[MEL_FILTERS + 2];
    for (int i = 0; i < MEL_FILTERS + 2; i++) {
        hz_points[i] = 700.0f * (powf(10.0f, mel_points[i] / 2595.0f) - 1.0f);
    }

    // 转换为FFT bin索引
    int bin_indices[MEL_FILTERS + 2];
    for (int i = 0; i < MEL_FILTERS + 2; i++) {
        bin_indices[i] = floorf((FFT_LEN + 1) * hz_points[i] / sample_rate);
    }

    // 构建三角形滤波器
    for (int m = 1; m <= MEL_FILTERS; m++) {
        for (int k = 0; k <= FFT_LEN / 2; k++) {
            if (k < bin_indices[m - 1]) {
                filters[m - 1][k] = 0.0f;
            } else if (k <= bin_indices[m]) {
                if (bin_indices[m] != bin_indices[m - 1]) {
                    filters[m - 1][k] = (float)(k - bin_indices[m - 1]) /
                                        (bin_indices[m] - bin_indices[m - 1]);
                }
            } else if (k <= bin_indices[m + 1]) {
                if (bin_indices[m + 1] != bin_indices[m]) {
                    filters[m - 1][k] = (float)(bin_indices[m + 1] - k) /
                                        (bin_indices[m + 1] - bin_indices[m]);
                }
            } else {
                filters[m - 1][k] = 0.0f;
            }
        }
    }
}

// 主MFCC计算函数
void compute_mfcc(uint16_t* input_frame, uint16_t* mfcc_out, float sample_rate) {
    // 0. 初始化汉明窗（只需一次）
    static int window_initialized = 0;
    if (!window_initialized) {
        init_hamming_window();
        window_initialized = 1;
    }

    float preemph_frame[FRAME_LEN];      // 预加重后的帧
    float windowed_frame[FRAME_LEN];     // 加窗后的帧
    float fft_real[FFT_LEN] = {0};       // FFT实部
    float fft_imag[FFT_LEN] = {0};       // FFT虚部
    float power_spectrum[FFT_LEN / 2 + 1]; // 功率谱

    // 1. 预加重处理
    preemph_frame[0] = (float)input_frame[0];
    for (int i = 1; i < FRAME_LEN; i++) {
        preemph_frame[i] = (float)input_frame[i] -
                           PREEMPHASIS_ALPHA * input_frame[i - 1];
    }

    // 2. 应用汉明窗
    for (int i = 0; i < FRAME_LEN; i++) {
        windowed_frame[i] = preemph_frame[i] * HAMMING_WINDOW[i];
    }

    // 3. 准备FFT输入（补零）
    for (int i = 0; i < FRAME_LEN; i++) {
        fft_real[i] = windowed_frame[i];
    }

    // 4. 执行FFT
    fft(fft_real, fft_imag, FFT_LEN);

    // 5. 计算功率谱（取前257点）
    for(int i = 0; i <= FFT_LEN / 2; i++){
        power_spectrum[i] = (fft_real[i] * fft_real[i] +
                            fft_imag[i] * fft_imag[i]) / FFT_LEN;
    }

    // 6. 创建梅尔滤波器组
    float** mel_filters = (float**)osal_malloc(MEL_FILTERS * sizeof(float*));
    for(int i = 0; i < MEL_FILTERS; i++){
        mel_filters[i] = (float*)osal_malloc((FFT_LEN / 2 + 1 )*sizeof(float));
    }
    create_mel_filters(sample_rate, mel_filters);

    // 7. 应用梅尔滤波器组
    float filter_energies[MEL_FILTERS] = {0};
    for(int m = 0; m < MEL_FILTERS; m++){
        for (int k = 0; k <= FFT_LEN / 2; k++) {
            filter_energies[m] += power_spectrum[k] * mel_filters[m][k];
        }
        // 避免log(0)
        if (filter_energies[m] < 1e-10) filter_energies[m] = 1e-10;
    }

    // 8. 取对数
    float log_energies[MEL_FILTERS];
    for(int m = 0; m < MEL_FILTERS; m++){
        log_energies[m] = logf(filter_energies[m]);
    }

    // 9. DCT变换获取MFCC系数
    for (int i = 0; i < 13; i++) {
        mfcc_out[i] = 0;
        for (int m = 0; m < MEL_FILTERS; m++) {
            mfcc_out[i] += log_energies[m] *
                          cos(PI * i * (m + 0.5f) / MEL_FILTERS);
        }
        // 能量归一化因子
        if (i == 0) {
            mfcc_out[i] *= sqrtf(1.0 / MEL_FILTERS);
        } else {
            mfcc_out[i] *= sqrtf(2.0 / MEL_FILTERS);
        }
    }

    // 10. 清理内存
    for (int i = 0; i < MEL_FILTERS; i++) {
    	osal_free(mel_filters[i]);
    }

    //osal_free(mel_filters);
}



void dataloader(uint16_t* raw_data){
	if(*raw_data == 65535) return;
	init_models();
	split_audio_into_blocks(raw_data);
    split_into_frames(audio_blocks);
    cal_per_energy(audio_blocks,72,400);
    frames_to_mfcc(audio_blocks,72,400);
    uint8_t result[4];
    compute_llr(audio_blocks,72,400,result);
    printf("0s~1.5s:%d\n",result[0]);
    printf("1.5s~3s:%d\n",result[1]);
    printf("3s~4.5s:%d\n",result[2]);
    printf("4.5s~6s:%d\n",result[3]);
}
//uint16_t test = 65535;
//dataloader(&test);
