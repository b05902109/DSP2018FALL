#include "hmm.h"
#include <math.h>

int sampleN, modelN, T;
int sample[MAX_SAMPLENUM][MAX_SEQ];
double delta[MAX_MODELNUM][MAX_SEQ][MAX_STATE];

void loadSample(const char *filename){
	int index = 0, len;
	FILE *fp = open_or_die( filename, "r");
	char Q[MAX_SEQ] = "";
	while( fscanf( fp, "%s", Q) > 0 ){
		if(T == 0)
			T = strlen(Q);
		for(int i = 0; i < T; i++){
			sample[index][i] = (int)(Q[i] - 'A');
		}
		index += 1;
	}
	sampleN = index;
	fclose(fp);
	return;
}

void evaluateDelta(HMM *hmm, int sample_idx){
	int stateN = hmm[0].state_num;
	double temp;
	for(int m = 0; m < modelN; m++){
		for(int t = 0; t < T; t++){
			for(int j = 0; j < stateN; j++){
				if(t == 0){
					delta[m][t][j] = hmm[m].initial[j] * hmm[m].observation[sample[sample_idx][t]][j];
				}
				else{
					temp = 0.0;
					for(int i = 0; i < stateN; i++){
						if(delta[m][t-1][i] * hmm[m].transition[i][j] > temp)
							temp = delta[m][t-1][i] * hmm[m].transition[i][j];
					}
					delta[m][t][j] = temp * hmm[m].observation[sample[sample_idx][t]][j];
				}
			}
		}
	}
}

int main(int argc, char const *argv[])
{
	HMM hmm_models[MAX_MODELNUM];
	int ans[MAX_SAMPLENUM], stateN;
	double maxP[MAX_SAMPLENUM];
	modelN = load_models(argv[1], hmm_models, MAX_MODELNUM);
	loadSample(argv[2]);
	stateN = hmm_models[0].state_num;
	for(int s = 0; s < sampleN; s++){
		evaluateDelta(hmm_models, s);
		maxP[s] = 0.0;
		for(int m = 0; m < modelN; m++){
			for(int i = 0; i < stateN; i++){
				if(delta[m][T-1][i] > maxP[s]){
					maxP[s] = delta[m][T-1][i];
					ans[s] = m;
				}
			}
		}
	}
	FILE *ansfp = open_or_die(argv[3], "w");
	for(int s = 0; s < sampleN; s++){
		fprintf(ansfp, "%s %e\n", hmm_models[ans[s]].model_name, maxP[s]);
	}
	fclose(ansfp);
	/*
	FILE *answerfp = open_or_die( "testing_answer.txt", "r");
	char Q[MAX_SEQ] = "";
	double correctN = 0.0;
	int idx = 0;
	while( fscanf( answerfp, "%s", Q) > 0 ){
		if((Q[7] - '1') == ans[idx])
			correctN += 1.0;
		idx += 1;
	}
	fclose(answerfp);
	*/
	//FILE *resultfp = open_or_die( "acc.txt", "w");
	//fprintf(resultfp, "%f\n", correctN / sampleN);
	//fclose(resultfp);
	//printf("%f\n", correctN / sampleN);
	
	return 0;
}