#pragma once
#include<string>
#include<vector>
#include<unordered_set>
#include<algorithm>
#include<iostream>
using std::string;
using std::vector;
using std::unordered_set;
using std::to_string;
//单词类型
enum WORD_TYPE
{
	TERMINAL = 0,
	VALUE = 1,
	DERIVATOR = 2,
	REDUCE = 3
};

//单词
class Word
{
public:
	int index = 0;
	WORD_TYPE type;
	string name;
	Word() {}
	Word(WORD_TYPE t, string s):type(t),name(s) {}
	bool operator==(const Word& word)
	{
		return word.name == name && word.type == type;
	}
	bool isType(WORD_TYPE t) { return t == type; }
};

class WordHash
{
public:
	size_t operator()(const Word& word) const
	{
		return std::hash<string>()(word.name) ^ std::hash<int>()(word.type);
	}
};
class WordCmp
{
public:
	bool operator()(const Word& word1, const Word& word2) const
	{
		return word1.name == word2.name && word1.type == word2.type;
	}
};

//文法
class Grammar
{
public:
	Grammar() {}
	Grammar(int i, const vector<Word>& g) :index(i),words(g){}
	int index = 2;//当前位置
	int id = 0;
	vector<Word> words;//文法
	vector<Word> reductions;//归约符号
	Word getNowWord() { return words[index]; }
	bool reduction() { return index == words.size(); }
	Word startWord() { return words[0]; }
	void setReduce()
	{
		int i = 0;
		while (i < words.size()&&!words[i].isType(REDUCE)) i++;
		int k = i++;
		if (k == words.size())
			return;
		for (int j = i; j < words.size(); j++)
			reductions.push_back(words[j]);
		int wsize = words.size();
		for (int j = k; j < wsize; j++)
		{
			words.pop_back();
		}
	}
	
};


class PairHash
{
public:
	size_t operator()(const std::pair<Word, int>& pii) const
	{
		return std::hash<string>()(pii.first.name) ^ std::hash<int>()(pii.first.type) ^ std::hash<int>()(pii.second);
	}
};

class PairCmp
{
public:
	bool operator()(const std::pair<Word, int>& pii1, const std::pair<Word, int>& pii2) const
	{
		return pii1.first.name == pii2.first.name && pii1.first.type == pii2.first.type && pii1.second == pii2.second;
	}
};

class Group
{
public:
	int id = 0;
	vector<Grammar> grammars;
	std::unordered_set<std::pair<Word, int>,PairHash ,PairCmp> connects;
	void pushConnect(const std::pair<Word, int>& ct) { connects.insert(ct); }
	void pushGrammar(const Grammar& g) { grammars.push_back(g); }
	int grammarCount() const{ return grammars.size(); }
	bool empty() { return grammars.empty(); }
};



/*文法组哈希函数，与比较同理*/
class GroupHash
{
public:
	size_t operator()(const Group& group) const
	{
		size_t result = 0;
		for (int i = 0; i < group.grammars.size(); i++)
		{
			result ^= group.grammars[i].index;
			for (int j = 0; j < group.grammars[i].words.size(); j++)
			{
				result ^= (std::hash<string>()(group.grammars[i].words[j].name) ^ std::hash<int>()(group.grammars[i].words[j].type));
			}
		}
		return result;
	}
};


/*文法组比较函数，要比较每个文法和当前正在处理的字符下标*/
class GroupCmp
{
public:
	bool operator()(const Group& group1, const Group& group2) const
	{
		if (group1.grammars.size() != group2.grammars.size())
			return false;
		for (int i = 0; i < group1.grammars.size(); i++)
		{
			if (group1.grammars[i].words.size() != group2.grammars[i].words.size())
				return false;
			if (group1.grammars[i].index != group2.grammars[i].index)
				return false;
			for (int j = 0; j < group1.grammars[i].words.size(); j++)
			{
				if (!(group1.grammars[i].words[j].name == group2.grammars[i].words[j].name &&
					group1.grammars[i].words[j].type == group2.grammars[i].words[j].type))
					return false;
			}
		}
		return true;
	}
};


class LR
{
	vector<string> strGrammars;//文法
	//unordered_set<Group, GroupHash, GroupCmp> findGroup;//Group表
	unordered_set<Group, GroupHash, GroupCmp> groupsSet;//Group表
	unordered_set<Word, WordHash, WordCmp> wordsSet;
	vector<Group> groups;
	Group standardGroup;
	vector<vector<string>> LRTable;

	Grammar scanGrammar(string s)
	{
		int i = 0;
		string t;
		vector<Word> g;
		while (i < s.size())
		{
			if (s[i] == ' ')
			{
				i++;
				continue;
			}
			else if (s[i] >= 'A' && s[i] <= 'Z')
			{
				while (s[i] >= 'A' && s[i] <= 'Z')
				{
					t += s[i];
					i++;
				}
				g.push_back({ TERMINAL,t });
				t = "";
			}
			else if (s[i] >= 'a' && s[i] <= 'z')
			{
				while (s[i] >= 'a' && s[i] <= 'z')
				{
					t += s[i];
					i++;
				}
				g.push_back({ VALUE,t });
				t = "";
			}
			else if (s.substr(i, 3) == "-->")
			{
				g.push_back({ DERIVATOR,"-->" });
				i += 3;
			}
			else if (s[i] == '+')
			{
				g.push_back({ VALUE,"+" });
				i++;
			}
			else if (s[i] == '-')
			{
				g.push_back({ VALUE,"-" });
				i++;
			}
			else if (s[i] == '*')
			{
				g.push_back({ VALUE,"*" });
				i++;
			}
			else if (s[i] == '/')
			{
				g.push_back({ VALUE,"/" });
				i++;
			}
			else if (s[i] == ';')
			{
				g.push_back({ VALUE,";" });
				i++;
			}
			else if (s[i] == '(')
			{
				g.push_back({ VALUE,"(" });
				i++;
			}
			else if (s[i] == ')')
			{
				g.push_back({ VALUE,")" });
				i++;
			}
			else if (s.substr(i, 3) == "|>>")
			{
				g.push_back({ REDUCE,"|>>" });
				i += 3;
			}
		}

		return { 2,g };
	}

	void init()
	{
		int index = 0;
		//分析出单词后，将文法存储到组中
		//cout << "  ";
		for (int i = 0; i < strGrammars.size(); i++)
		{
			//提取文法中的单词
			Grammar grammar = scanGrammar(strGrammars[i]);
			grammar.id = i;
			for (int j = 0; j < grammar.words.size(); j++)
			{
				if (!grammar.words[j].isType(DERIVATOR) && !grammar.words[j].isType(REDUCE))//如果不是-->符号则存储
				{
					if (wordsSet.find(grammar.words[j]) == wordsSet.end())//这里收集所有的单词
					{
						grammar.words[j].index = index++;//单词计数
						wordsSet.insert(grammar.words[j]);//插入新单词
						//cout << grammar.words[j].name << " ";
					}
				}
			}
			grammar.setReduce();
			standardGroup.pushGrammar(grammar);
		}
		//将生成好的group存入数组和集合中
		groups.push_back(standardGroup);
		groupsSet.insert(standardGroup);
	}

	void generateLR()
	{
		int i = 0;
		while (i < groups.size())
		{
			for (auto word : wordsSet)
			{
				Group tempGroup;
				for (int j = 0; j < groups[i].grammarCount(); j++)
				{
					Grammar grmmar = groups[i].grammars[j];
					//如果是归约的语法则不在继续分析
					if (grmmar.reduction())
						continue;
					if (grmmar.getNowWord() == word)
					{
						//把文法添加到下一个组里
						grmmar.index++;
						tempGroup.pushGrammar(grmmar);
						//把非终结符关联的也加入，对于下一个刚好归约的则不在分析有非终结符的情况
						if (!grmmar.reduction())
							for (int k = 0; k < standardGroup.grammarCount(); k++)
								if (grmmar.getNowWord() == standardGroup.grammars[k].startWord() && standardGroup.grammars[k].startWord().isType(TERMINAL))
									tempGroup.pushGrammar(standardGroup.grammars[k]);
					}
				}
				//这里要查找组是否有重复
				if (!tempGroup.empty())
				{
					auto f = groupsSet.find(tempGroup);
					//如果找到了相同的组
					if (f != groupsSet.end())
					{
						groups[i].pushConnect({ word,f->id });
					}
					else
					{
						tempGroup.id = groups.size();
						groups.push_back(tempGroup);
						groupsSet.insert(tempGroup);
						groups[i].pushConnect({ word,tempGroup.id });
					}
				}
			}
			i++;
		}
	}

	void generateLRTable()
	{
		LRTable.resize(groups.size());
		for (int i = 0; i < groups.size(); i++)
		{
			LRTable[i].resize(wordsSet.size());
		}
		//初始化LR表
		for (int i = 0; i < groups.size(); i++)
			for (int j = 0; j < wordsSet.size(); j++)
				LRTable[i][j] = "e";
		//将生成好的图转换成LR表
		for (int i = 0; i < groups.size(); i++)
		{
			for (auto con : groups[i].connects)
			{
				if (con.first.type == VALUE)
					LRTable[i][wordsSet.find(con.first)->index] = "s" + to_string(con.second+1);
				else if (con.first.type == TERMINAL)
				{
					int k = wordsSet.find(con.first)->index;
					LRTable[i][wordsSet.find(con.first)->index] = "g" + to_string(con.second+1);
				}
			}
			for (auto gra : groups[i].grammars)
			{
				if (gra.reduction())//如果此时需要归约
				{
					for (int j = 0; j < gra.reductions.size(); j++)
					{
						LRTable[i][wordsSet.find(gra.reductions[j])->index] = "r" + to_string(gra.id);
					}
				}
			}
		}
	}

	
public:
	LR() {}
	void setGrammars(vector<string> gs) { strGrammars = gs; }
	vector<vector<string>> getLR()
	{
		init();
		generateLR();
		generateLRTable();
		return LRTable;
	}
	void exportLRTable()
	{
		vector<Word> headWords;
		for (auto w : wordsSet)
			headWords.push_back(w);
		//排序
		sort(headWords.begin(), headWords.end(), [](const Word& a, const Word& b) {return a.index < b.index; });

		//输出头
		for (auto h : headWords)  std::cout << h.name << '\t';
		std::cout << std::endl;
		//输出表内容
		for (int i = 0; i < LRTable.size(); i++)
		{
			for (int j = 0; j < LRTable[i].size(); j++)
				std::cout << LRTable[i][j] <<'\t';
			std::cout << std::endl;
		}

	}
};
