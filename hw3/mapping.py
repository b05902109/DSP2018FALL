import sys

if __name__ == '__main__':
	ZhuYin_big5 = dict()
	with open(sys.argv[1], 'r', encoding='BIG5-HKSCS') as fp:
		for line in fp:
			if line[2] in ZhuYin_big5:
				ZhuYin_big5[line[2]] = ZhuYin_big5[line[2]] + line[0]
			else:
				ZhuYin_big5[line[2]] = line[0]
	with open(sys.argv[2], 'w', encoding='BIG5-HKSCS') as fp:
		for item in ZhuYin_big5.items():
			print('%s\t%s'%(item[0], " ".join(item[1])), file=fp)
			for word in item[1]:
				print('%s\t%s'%(word, word), file=fp)