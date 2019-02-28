#include "hmm.h"
#include <math.h>

int T = 0;
int sampleN = 0;
int sample[MAX_SAMPLENUM][MAX_SEQ];
double alpha[MAX_SAMPLENUM][MAX_SEQ][MAX_STATE];
double beta[MAX_SAMPLENUM][MAX_SEQ][MAX_STATE];
double Gamma[MAX_SAMPLENUM][MAX_SEQ][MAX_STATE];
double epsilon[MAX_SAMPLENUM][MAX_SEQ][MAX_STATE][MAX_STATE];

void loadSample(const char *filename){
	int index = 0, len;
	FILE *fp = open_or_die( filename, "r");
	if(fp == NULL)
		printf("loadSample fail.\n");
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

void evaluateAlpha(HMM *hmm){
	//printf("alpha\n");
	int stateN = hmm->state_num;
	double temp;
	for(int n = 0; n < sampleN; n++){
		for(int t = 0; t < T; t++){
			for(int j = 0; j < stateN; j++){
				if(t == 0){
					alpha[n][t][j] = ( hmm->initial[j] ) * ( hmm->observation[sample[n][t]][j] );
				}
				else{
					temp = 0.0;
					for(int i = 0; i < stateN; i++){
						temp += alpha[n][t - 1][i] * ( hmm->transition[i][j] );
					}
					alpha[n][t][j] = temp * ( hmm->observation[sample[n][t]][j] );
				}
			}
		}
	}
}

void evaluateBeta(HMM *hmm){
	int stateN = hmm->state_num;
	double temp;
	for(int n = 0; n < sampleN; n++){
		for(int t = T - 1; t >= 0; t--){
			for(int i = 0; i < stateN; i++){
				if(t == T - 1){
					beta[n][t][i] = 1.0;
				}
				else{
					temp = 0.0;
					for(int j = 0; j < stateN; j++){
						temp += ( hmm->transition[i][j] ) * ( hmm->observation[sample[n][t + 1]][j] ) * beta[n][t + 1][j];
					}
					beta[n][t][i] = temp;
				}
			}
		}
	}
}

void evaluateGamma(HMM *hmm){
	int stateN = hmm->state_num;
	double temp;
	for(int n = 0; n < sampleN; n++){
		for(int t = 0; t < T; t++){
			temp = 0.0;
			for(int idx = 0; idx < stateN; idx++){
				temp += alpha[n][t][idx] * beta[n][t][idx];
			}
			for(int i = 0; i < stateN; i++){
				Gamma[n][t][i] = alpha[n][t][i] * beta[n][t][i] / temp;
			}
		}
	}
}

void evaluateEpsilon(HMM *hmm){
	int stateN = hmm->state_num;
	double temp;
	for(int n = 0; n < sampleN; n++){
		for(int t = 0; t < T - 1; t++){
			temp = 0.0;
			for(int idx_i = 0; idx_i < stateN; idx_i ++)
				for(int idx_j = 0; idx_j < stateN; idx_j ++)
					temp += alpha[n][t][idx_i] * (hmm->transition[idx_i][idx_j]) * (hmm->observation[sample[n][t+1]][idx_j]) * beta[n][t + 1][idx_j];
			for(int i = 0; i < stateN; i++){
				for(int j = 0; j < stateN; j++){
					epsilon[n][t][i][j] = alpha[n][t][i] * (hmm->transition[i][j]) * (hmm->observation[sample[n][t+1]][j]) * beta[n][t + 1][j] / temp;
				}
			}
		}
	}
}

void update(HMM *old, HMM *new, const char *name){
	new->model_name = (char *)malloc( sizeof(char) * (strlen(name)+1));
	strcpy(new->model_name, name);
	new->observ_num = old->observ_num;
	new->state_num = old->state_num;
	int stateN = old->state_num, observN = old->observ_num;
	double Gamma_temp, epsilon_temp, Gamma_temp2;
	//pi
	for(int i = 0; i < stateN; i++){
		Gamma_temp = 0.0;
		for(int n = 0; n < sampleN; n++){
			Gamma_temp += Gamma[n][0][i];
		}
		new -> initial[i] = Gamma_temp / sampleN;
	}
	//a_ij
	for(int i = 0; i < stateN; i++){
		Gamma_temp = 0.0;
		for(int n = 0; n < sampleN; n++){
			for(int t = 0; t < T - 1; t ++){
				Gamma_temp += Gamma[n][t][i];
			}
		}
		for(int j = 0; j < stateN; j ++){
			epsilon_temp = 0.0;
			for(int n = 0; n < sampleN; n++){
				for(int t = 0; t < T - 1; t ++){
					epsilon_temp += epsilon[n][t][i][j];
				}
			}
			new -> transition[i][j] = epsilon_temp / Gamma_temp;
		}
	}
	//b
	for(int j = 0; j < stateN; j++){
		double p[observN], p2 = 0.0;
		for(int i = 0; i < observN; i++)
			p[i] = 0.0;
		for(int n = 0; n < sampleN; n++){
			for(int t = 0; t < T; t++){
				p[sample[n][t]] += Gamma[n][t][j];
				p2 += Gamma[n][t][j];
			}
		}
		for(int k = 0; k < observN; k++)
			new->observation[k][j] = p[k] / p2;
	}
	/*
	for(int k = 0; k < observN; k++){
		for(int j = 0; j < stateN; j ++){
			Gamma_temp = 0.0;
			Gamma_temp2 = 0.0;
			for(int n = 0; n < sampleN; n++){
				for(int t = 0; t < T; t++){
					if(sample[n][t] == k){
						Gamma_temp += Gamma[n][t][j];
					}
					Gamma_temp2 += Gamma[n][t][j];
				}
			}
			//printf("%f %f\n", Gamma_temp, Gamma_temp2);
			new -> observation[k][j] = Gamma_temp / Gamma_temp2;
		}
	}
	*/
}

void print(HMM *hmm_initial, int type){
	for(int t = 0; t < T; t++){
		if(type == 4){
			printf("%d", sample[0][t]);
		}
		else{
			for(int n = 0; n < hmm_initial->state_num; n++){
				if(type == 0)
					printf("%f ", alpha[0][t][n]);
				else if(type == 1)
					printf("%f ", beta[0][t][n]);
				else if(type == 2)
					printf("%f ", Gamma[0][t][n]);
				else if(type == 3 && t != T - 1)
					printf("%f ", epsilon[0][t][0][n]);
			}
			printf("\n");
		}
	}
	printf("------\n");
}

int main(int argc, char const *argv[])
{
	int iteration = atoi(argv[1]);
	HMM hmm_initial, hmm_update;
	loadHMM(&hmm_initial, argv[2]);
	//dumpHMM(stderr, &hmm_initial);
	loadSample(argv[3]);
	for(int iter = 0; iter < iteration; iter ++){
		//printf("------ %d iteration ------\n", iter);
		//dumpHMM(stderr, &hmm_initial);
		evaluateAlpha(&hmm_initial);
		evaluateBeta(&hmm_initial);
		evaluateGamma(&hmm_initial);
		evaluateEpsilon(&hmm_initial);
		//print(&hmm_initial, 4);
		//print(&hmm_initial, 0);
		//print(&hmm_initial, 1);
		//print(&hmm_initial, 2);
		//print(&hmm_initial, 3);
		update(&hmm_initial, &hmm_update, argv[4]);
		//dumpHMM(stderr, &hmm_update);
		free(hmm_initial.model_name);
		hmm_initial = hmm_update;
	}
	//dumpHMM(stderr, &hmm_update);
	FILE *modelfp = open_or_die(argv[4], "w");
	dumpHMM(modelfp, &hmm_update);
	fclose(modelfp);
	return 0;
}