/*
 * algo.h
 *
 *  Created on: 2025年6月15日
 *      Author: wodefa
 */

#ifndef GALAXY_SDK_ALGO_H_
#define GALAXY_SDK_ALGO_H_
void dataloader(uint16_t* raw_data);
void frames_to_mfcc(uint8_t** audio_blocks, int num_frames, int frame_length);
//void compute_mfcc(uint16_t* input_frame, uint16_t* mfcc_out, float sample_rate);


#endif /* GALAXY_SDK_ALGO_H_ */
