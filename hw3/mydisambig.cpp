#include <stdio.h>
#include <string.h>
#include "Ngram.h"
#include "VocabMap.h"
#include "Vocab.h"

#define MAX_LINE_NUM 	100
#define MAX_LINE_LENGTH 512
#define MAX_CHOICE 		1024

int main(int argc, char const *argv[])
{
	/* ---- bigram create ---- */
	//printf("%s %s %s\n", argv[6], argv[4], argv[2]);
	int ngram_order = atoi(argv[8]);
	Vocab voc, zhuyin, big5;
	
	Ngram bigram(voc, ngram_order);
	{
		File bigramFile(argv[6], "r");
		bigram.read(bigramFile);
		bigramFile.close();
	}
	
	/* ---- word_map create ---- */
	VocabMap my_wordmap(zhuyin, big5);
	{
		File wordmap_File(argv[4], "r");
		my_wordmap.read(wordmap_File);
		wordmap_File.close();
	}
	big5.addWord("<s>");
    big5.addWord("</s>");

	/* ---- input file get content ----*/
	File input_File(argv[2], "r");
	char lines[MAX_LINE_NUM][MAX_LINE_LENGTH], *read_line;
	int line_num = 0;
	while (read_line = input_File.getline()){
		strcpy(lines[line_num ++], read_line);
	}
	input_File.close();
	
	/* ---- Viterbi ---- */
	VocabIndex Unknown_idex = voc.getIndex(Vocab_Unknown);
	for(int line_idx = 0; line_idx < line_num; line_idx ++){
		VocabString line[MAX_LINE_LENGTH + 2];
		//VocabIndex line_index[MAX_LINE_LENGTH + 2];
		VocabIndex choice_index[MAX_LINE_LENGTH + 2][MAX_CHOICE];
		LogP dp_table[MAX_LINE_LENGTH + 2][MAX_CHOICE];
		int back_tracking[MAX_LINE_NUM + 2][MAX_LINE_LENGTH];
		int choice[MAX_LINE_LENGTH + 2];

		/* ---- convert words to VocabIndex ---- */
		unsigned line_length = Vocab::parseWords(lines[line_idx], &line[1], MAX_LINE_LENGTH);
		//line[0] = "<s>";
		//line[line_length + 1] = "</s>";
//cout << line << "\n";
//cout << line_length << "\n";

		/* ---- find Jhuyin' s possible choices ---- */
		choice[0] = 1;
		choice[line_length + 1] = 1;
		choice_index[0][0] = big5.getIndex("<s>");
		choice_index[line_length + 1][0] = big5.getIndex("</s>");
		for (int i = 0; i < line_length; i++){
//cout << i << " ";
			VocabIndex index;
			Prob temp;	//just something for call the function, useless
			int count = 0;
			VocabMapIter iter(my_wordmap, zhuyin.getIndex(line[i + 1]));
			iter.init();
			while(iter.next(index, temp)){
				choice_index[i + 1][count++] = index;
			}
			choice[i + 1] = count;
		}
/*
cout << "\n";
for(int i = 0; i < line_length; i++)
	cout << choice[i] << " ";
cout << "\n";
*/
		/* ---- start dp(Viterbi) ---- */
		VocabString word_choice_now, word_choice_past;
		VocabIndex index_choice_now, index_choice_past;
		LogP tempP;
		dp_table[0][0] = 0.0;
		for(int now = 1; now < line_length + 2; now++){
//cout << now << " ";
			for(int choice_now = 0; choice_now < choice[now]; choice_now++){
				LogP maxP = -1000000;
				word_choice_now = big5.getWord(choice_index[now][choice_now]);
				index_choice_now = voc.getIndex(word_choice_now);
				if(index_choice_now == Vocab_None)
					index_choice_now = Unknown_idex;
				for(int choice_past = 0; choice_past < choice[now - 1]; choice_past ++){
					word_choice_past = big5.getWord(choice_index[now - 1][choice_past]);
					index_choice_past = voc.getIndex(word_choice_past);
					if(index_choice_past == Vocab_None)
						index_choice_past = Unknown_idex;
					VocabIndex context[] = {index_choice_past, Vocab_None};
					tempP = dp_table[now - 1][choice_past] + bigram.wordProb(index_choice_now, context);
					if(tempP > maxP){
						back_tracking[now][choice_now] = choice_past;
						maxP = tempP;
//cout << word_choice_now << "\n";
					}
				}
				dp_table[now][choice_now] = maxP;
			}
		}
//cout << "\n";
		/* ---- back tracking ---- */
		VocabString answer[MAX_LINE_LENGTH + 2];
		answer[0] = "<s>";
		answer[line_length + 1] = "</s>";
		int past_index = 0;
		for(int track_index = line_length; track_index > 0; track_index --){
			answer[track_index] = big5.getWord(choice_index[track_index][past_index]);
			past_index = back_tracking[track_index][past_index];
		}

		/*---- write out answer ---- */
		for( int i = 0; i <= line_length; i++ )
			printf("%s ", answer[i]);
		printf("</s>\n");
		//exit(1);
	}
	return 0;
}