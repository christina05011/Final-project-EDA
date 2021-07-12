#include <iostream>
#include <windows.h>
#include <vector>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <string>
#include <chrono>
using namespace std;
/// <summary>
/// Implementation of Radix Tree, only with functions: insert and find.
/// Implementation for Information Retrieval.
/// Using TF-idf to know the similarity.
/// Function: TF*IDF
	//TF = Number of ocurrences of word in document ---> " labels[n_doc].second "
	//IDF = log(N/df)
	//N = Total number of doc in corpus  ---> " id_docs.size() "
	//df = Number of documents containing the word ---> " labels.size() "
/// Dataset (and some documents to run this code) to download from: https://drive.google.com/drive/folders/1GyTJs0WuUFWIh7CDhELWKohfRXn02Jq4?usp=sharing
/// Dataset arxiv-metadata-oai-snapshot.json to download from: https://www.kaggle.com/Cornell-University/arxiv
/// Documents with included "Clear" in their names, were preprocessed by python code. (.py(s) are included in this repository)
/// </summary>
void readJSON() { //To reduce with: "[id] abstract" into .txt (only one .txt file)
	ifstream read("arxiv-metadata-oai-snapshot.json"); //Read data .json 
	//number of abstracts = 1901600
	if (read.fail()) { cout << "ERROR: Don't open file!" << endl; return; }
	string line, id, _abstract;
	ofstream out("documents.txt"); //to put data in another .txt
	int i = 1;
	while (getline(read, line)) {
		cout << i << endl; i++;
		id = line.substr(0, line.find(',')); //{"id":"..."
		id = id.substr(7); //..."
		id = id.substr(0, id.size() - 1); //...
		line = line.substr(line.find("abstract"));
		_abstract = line.substr(0, line.find("\"versions\":")); //abstract":"  ... .\n","
		_abstract = _abstract.substr(13); //... .\n","
		_abstract = _abstract.substr(0, _abstract.size() - 5); //... .
		out << "[" << id << "] " << _abstract << endl; //"[id] abstract"
	}
	read.close(); out.close();
}
void preprocessing(string document, vector<string>& clear) { //Preprocessing: Remove dirty data from only a document_string
	//And return with a vector with all strings clear
	//Verify if has punctuation //Verify all that are not alphabetic //Convert all to lowercase //Verify if is a stopword 
	string wordClean = "", wordSW; //word to read from stopwrods file
	int i = 0, s_l = (int)document.size(), count_not_SW, count_line_SW;
	while (i < s_l) {
		//if is '\n...' delete '\n'
		if (document[i] == '\\' && i + 1 < s_l && document[i + 1] == 'n') { wordClean += ' '; i += 2; }
		//if is ('_' || '-' || ' ') => ' '
		else if (document[i] == '_' || document[i] == '-' || document[i] == ' ') { wordClean += ' '; i++; }
		//if is alphabetic //Convert all to lowercase 
		else if (isalpha(document[i])) { wordClean += (char)tolower(document[i]); i++; }
		//if is not-alphabetic
		else i++;
		//If wordClean is a word...
		if (!wordClean.empty() && wordClean.back() == ' ') { // wordClean ends with ' '
			//Verify if is a stopword
			wordClean = wordClean.substr(0, wordClean.size() - 1); //Erase last space
			if (!wordClean.empty()) { //If word is not empty (maybe without last space will be empty)
				ifstream r_SW("stopwordsEnglish.txt"); //read stopwords file 
				if (r_SW.fail()) { cout << "ERROR: Don't open file!" << endl; exit(1); }
				count_not_SW = count_line_SW = 0;
				while (getline(r_SW, wordSW)) { //Read line by line from the stopwords file (each stopwords in a line)
					if (wordSW != wordClean) count_not_SW++; //if the word is not equal than this stopword, increase counter
					count_line_SW++;
				} //If number of stopwords is equal than counter (number of not stopwords), is NOT a stopword
				if (count_line_SW == count_not_SW) clear.push_back(wordClean); //The word is not a stopword, so save to clear_vector
				r_SW.close();
			}
			wordClean = "";
		}
	}//LAST WORD, maybe didn't end with space, so verify
	if (!wordClean.empty()) { // If wordClean is NOT empty
		//Verify if is a stopword
		ifstream r_SW("stopwordsEnglish.txt"); //read stopwords file 
		if (r_SW.fail()) { cout << "ERROR: Don't open file!" << endl; exit(1); }
		count_not_SW = count_line_SW = 0;
		while (getline(r_SW, wordSW)) { //Read line by line from the stopwords file (each stopwords in a line)
			if (wordSW != wordClean) count_not_SW++; //if the word is not equal than this stopword, increase counter
			count_line_SW++;
		} //If number of stopwords is equal than counter (number of not stopwords), is NOT a stopword
		if (count_line_SW == count_not_SW) clear.push_back(wordClean); //The word is not a stopword, so save to clear_vector
		r_SW.close();
	}
}
struct RadixNode {
	string radix; //root OR part of word OR word 
	vector<RadixNode*> nodes; // the rest of word(s) --> (like child nodes)
	vector<pair<string, int>> labels; //1° string: id_document where is this word, 2° int: how many times this word is repeated in that document.

	RadixNode(string rad) : radix(rad) {}
	~RadixNode() {}

	void addInLabels(string id_doc) { //Add id_doc in RadixNode_labels 
		//First: Find id_doc in labels.first
		int s_l = int(labels.size());
		for (int i = 0; i < s_l; i++) {
			if (labels[i].first == id_doc) {
				labels[i].second++; //encrase number_word_repeated in doc
				return; //finish add
			}
		} //If not found
		labels.push_back(pair<string, int>(id_doc, 1)); //make new doc_label
	}
};
struct RadixTree {
	RadixNode* root;
	vector<string> id_docs; //here is the number AND id of docs inserted
	RadixTree() {
		root = new RadixNode(""); //to start all the tree AND it is not part of words
	}
	~RadixTree() { deleteTree(root); }
	void deleteTree(RadixNode* n) { //Part of destroyer
		if (!n) return;
		for (int i = n->nodes.size() - 1; i >= 0; i--)
			deleteTree(n->nodes[i]);
		delete n; n = NULL;
		return;
	}
	int findDoc(string id_doc) { //Find doc in id_docs_vector ( function for scoreDoc() ) //return pos
		int s = (int)id_docs.size();
		for (int i = 0; i < s; i++) {
			if (id_docs[i] == id_doc) return i; //found
		}
		return -1; //not found
	}
	bool findWord(string& word, RadixNode*& dir, string& rad) { //If word exists in the tree 
		//word = query_word; dir = RadixNode where the word is OR maybe where will be inserted; rad = root/radix in common
		int s_n = (int)dir->nodes.size(), s_w = (int)word.size(), p, s_r;
		for (int j = 0; j < s_n; j++) { //Verify all RadixNode_nodes
			if (dir->nodes[j]->radix[0] == word[0]) { //If first letter is equal
				dir = dir->nodes[j];
				if (dir->radix == word) return 1; //If find the exactly word
				rad = word[0]; //radix is equal to the first letter
				//Add to radix while letters are equal...
				p = 1; s_r = (int)dir->radix.size();
				while (p < s_w && p < s_r) {
					if (dir->radix[p] == word[p]) { rad += word[p]; p++; }
					else break;
				}
				word.erase(word.begin(), word.end() - s_w + p); //query_word erase the radix in common //Number of equal letters = radix size
				string radixTemp = dir->radix.substr(p, s_r - p); //string without common radix
				//If word is empty, found THE word inside this word AND then to modify //OR If RadixNode doesn't have nodes, not found word 
				//OR word and radixTemp are not empty, means both have common radix and that's where it has to be modified
				if (dir->nodes.empty() || word.empty() || (!word.empty() && !radixTemp.empty())) return 0;
				if (findWord(word, dir, rad)) return 1; //continue searching ...
				return 0; //If not found
			}
		} return 0; //If not found
	}
	RadixNode* insertWord(string word) { //Insert word in the tree //word = query_word
		RadixNode* dir = root, * end; string rad;
		if (findWord(word, dir, rad)) return dir; //already exists & returns end_word with *dir
		//Not exists the word OR is inside other word OR have the same common radix but are not equal words ...
		//If the word is empty, do not create new RadixNode, cause the query_word is part of *dir radix
		if (!word.empty()) end = new RadixNode(word); //word = query_word without common radix
		else end = dir; //query_word is part of *dir radix
		if (dir != root && rad != dir->radix) { //If not have the same radix/root, make new children/rest of words

			string rest = dir->radix; //rest = radix_*dir without common radix
			rest = rest.substr(rad.size()); //erase common radix 
			RadixNode* child = new RadixNode(rest); //past in this RadixNode the nodes_vector of *dir
			child->labels = dir->labels; //equal vector_labels
			child->nodes = dir->nodes; //equal vector_nodes

			dir->nodes.clear(); //clear nodes_vector of *dir
			dir->labels.clear(); //clear labels_vector of *dir
			dir->nodes.push_back(child); //now *dir ONLY has *child
			dir->radix = rad; //*dir has new radix/root
		}
		if (!word.empty()) { //If word is not empty, push new RadixNode with the word (rest of query_word without common radix)
			dir->nodes.push_back(end);
		} //If word is empty, not good to insert end (= *dir)
		return end; //return end_word = *end
	}
	void print(RadixNode* n) { //Print tree with content of words
		if (n->nodes.empty()) return;
		cout << "print " << n->radix << endl;
		int s = int(n->nodes.size()), i;
		for (i = 0; i < s; i++) {
			cout << n->nodes[i]->radix << "	";
		} cout << endl;
		for (i = 0; i < s; i++) {
			print(n->nodes[i]);
		} cout << endl;
	}
	void printLabels(RadixNode* n) { //Print tree with RadixNode_labels and content of words
		if (n->nodes.empty()) return;
		cout << "print " << n->radix << endl;
		int s = int(n->nodes.size()), i, ss;
		for (i = 0; i < s; i++) {
			cout << n->nodes[i]->radix << ":	";
			ss = int(n->nodes[i]->labels.size());
			for (int j = 0; j < ss; j++)
				cout << n->nodes[i]->labels[j].first << " " << n->nodes[i]->labels[j].second << ", ";
			cout << endl;
		} cout << endl;
		for (i = 0; i < s; i++) {
			printLabels(n->nodes[i]);
		} cout << endl;
	}
	//Insert doc, already preprocessed
	void insertDoc(string document) { //insert word by word of the document 
		//Remember document is this form: [id] abstract
		string id_doc = document.substr(1, document.find(']') - 1); //Get id_doc, from [id_doc] to id_doc
		document = document.substr(document.find(' ') + 1); //Remove [id_doc], get only abstract
		id_docs.push_back(id_doc); //insert id_doc to vector_id_docs, increase number of docs with that insert
		RadixNode* t; string word; bool run = 1;
		while (run) { //all words are separated by space
			if (document.find(' ') == std::string::npos) { word = document; run = 0; }
			else word = document.substr(0, document.find(' '));
			t = insertWord(word); //*t is RadixNode* where the word was inserted
			t->addInLabels(id_doc); //Add in RadixNode_labels 
			document = document.substr(document.find(' ') + 1);
		} t = 0; delete t; //delete temp t
	}
	//Insert doc, but first read .txt (preprocessing)
	void insertDocWthPre(string document) { //insert word by word of the document 
		//Remember document is this form: [id] abstract
		string id_doc = document.substr(1, document.find(']') - 1); //Get id_doc, from [id_doc] to id_doc
		document = document.substr(document.find(' ') + 1); //Remove [id_doc], get only abstract
		vector<string> prep; //vector with clean words from document 
		preprocessing(document, prep); //preprocessing abstract of document //return clear document
		id_docs.push_back(id_doc); //insert id_doc to vector_id_docs, increase number of docs with that insert
		RadixNode* t;
		int i = 0, s_d = (int)prep.size();
		for (; i < s_d; i++) {
			t = insertWord(prep[i]); //*t is RadixNode* where the word was inserted
			t->addInLabels(id_doc); //Add in RadixNode_labels 
		} t = 0; delete t; //delete temp t
	}
	void scoreDocWthPre(string query) { //Similarity per doc (for all docs inserted in the tree) //With preprocessing query_doc //To be preprocessed
		//FIRST: preprocess query doc
		ifstream read(query + ".txt");
		if (read.fail()) { cout << "ERROR: Don't open file!" << endl; return; }
		string query_doc, r_ln;
		while (getline(read, r_ln)) {
			query_doc += r_ln + ' ';
		}
		read.close();
		vector<string> prep; //vector with clean words from document
		preprocessing(query_doc, prep); //preprocessing query document //return clear document
		//SECOND: calculate TF*IDF
		ofstream out(query + "AllScore.csv"); //New .csv with score word by word (as a table)
		//To print first line of table .csv with all TF-IDF
		out << "Word,TF-IDF" << endl; //TF-IDF = TF*IDF
		//To print all ID documents
		out << ',';
		int i, s_docs = (int)id_docs.size() - 1, i_query = 0, s_query = (int)prep.size();
		for (i = 0; i < s_docs; i++)
			out << '\'' << id_docs[i] << "\',"; //print like this: 'name_doc'
		out << '\'' << id_docs[s_docs] << "\'" << endl;
		//
		string rad; RadixNode* end_word; float _idf;
		//To print al TF-IDF per doc
		int pos_id_docs, last, temp, s_labels;
		for (; i_query < s_query; i_query++) { //for preprocessing words
			end_word = root;
			out << prep[i_query] + ','; //preprocessing word
			if (!findWord(prep[i_query], end_word, rad)) { //If the word didn't find in the tree, print all docs with 0 (TF-IDF)
				for (i = 0; i < s_docs; i++)
					out << to_string(0.0) + ',';
				out << to_string(0.0) << endl;
				continue;
			}
			//If the word is found, calculate IDF & print TF-IDFs
			_idf = float(log10(float(s_docs + 1) / float(end_word->labels.size())));
			//TF-IDF per doc in labels_vector from end_word RadixNode 
			//In this part, calculate number of documents between each pair of documents from labels_vector
			//and print documents where are not in labels_vector, print 0.0 (TF-IDF)
			//means, number of documents between, print 0.0 (TF-IDF)
			last = -1, s_labels = end_word->labels.size() - 1;
			for (i = 0; i < s_labels; i++) {
				pos_id_docs = temp = findDoc(end_word->labels[i].first) - 1;
				while (pos_id_docs > last) {
					out << to_string(0.0) + ',';
					pos_id_docs--;
				}
				out << to_string(float(end_word->labels[i].second) * _idf) + ',';
				last = temp + 1;
			}//Last label
			pos_id_docs = temp = findDoc(end_word->labels[s_labels].first) - 1;
			while (pos_id_docs > last) {
				out << to_string(0.0) + ',';
				pos_id_docs--;
			}
			out << to_string(float(end_word->labels[s_labels].second) * _idf);
			//If the last label is or not the final document from id_docs_vector, verify as the above mentioned and print
			last = temp + 1; temp = s_docs;
			while (temp > last) {
				out << ',' + to_string(0.0);
				temp--;
			}
			out << endl;
		}
		out.close();
	}
	void scoreDoc(string query) { //Similarity per doc (for all docs inserted in the tree) //Without preprocessing  //Already preprocessed FILE .txt
		ifstream read(query + "Clear.txt");
		if (read.fail()) { cout << "ERROR: Don't open file!" << endl; return; }
		string query_doc;
		getline(read, query_doc); //All queryClear in one line
		read.close();
		//Calculate TF*IDF
		ofstream out(query + "AllScore.csv"); //New .csv with score word by word (like a table)
		//To print first line of table .csv with all TF-IDF
		out << "Word,TF-IDF" << endl; //TF-IDF = TF*IDF
		//To print all ID documents
		out << ',';
		int i, s_docs = (int)id_docs.size() - 1;
		for (i = 0; i < s_docs; i++)
			out << '\'' << id_docs[i] << "\',"; //print like this: 'name_doc'
		out << '\'' << id_docs[s_docs] << "\'" << endl;
		//
		string rad; RadixNode* end_word; float _idf;
		//To print al TF-IDF per doc
		int pos_id_docs, last, temp, s_labels; string word; bool run = 1;
		while (run) { //for preprocessed words
			if (query_doc.find(' ') == std::string::npos) { word = query_doc; run = 0; }
			else word = query_doc.substr(0, query_doc.find(' '));
			query_doc = query_doc.substr(query_doc.find(' ') + 1);
			end_word = root;
			out << word + ','; //preprocessing word
			if (!findWord(word, end_word, rad)) { //If the word didn't find in the tree, print all docs with 0 (TF-IDF)
				for (i = 0; i < s_docs; i++)
					out << to_string(0.0) + ',';
				out << to_string(0.0) << endl;
				continue;
			}
			//If the word is found, calculate IDF & print TF-IDFs
			_idf = float(log10(float(s_docs + 1) / float(end_word->labels.size())));
			//TF-IDF per doc in labels_vector from end_word RadixNode 
			//In this part, calculate number of documents between each pair of documents from labels_vector
			//and print documents where are not in labels_vector, print 0.0 (TF-IDF)
			//means, number of documents between, print 0.0 (TF-IDF)
			last = -1, s_labels = end_word->labels.size() - 1;
			for (i = 0; i < s_labels; i++) {
				pos_id_docs = temp = findDoc(end_word->labels[i].first) - 1;
				while (pos_id_docs > last) {
					out << to_string(0.0) + ',';
					pos_id_docs--;
				}
				out << to_string(float(end_word->labels[i].second) * _idf) + ',';
				last = temp + 1;
			}//Last label
			pos_id_docs = temp = findDoc(end_word->labels[s_labels].first) - 1;
			while (pos_id_docs > last) {
				out << to_string(0.0) + ',';
				pos_id_docs--;
			}
			out << to_string(float(end_word->labels[s_labels].second) * _idf);
			//If the last label is or not the final document from id_docs_vector, verify as the above mentioned and print
			last = temp + 1; temp = s_docs;
			while (temp > last) {
				out << ',' + to_string(0.0);
				temp--;
			}
			out << endl;
		}
		out.close();
	}
	void rankingDocFile(string query) { //Similarity per doc (for all docs inserted in the tree) //Without preprocessing 
		ifstream read(query + "Clear.txt");
		if (read.fail()) { cout << "ERROR: Don't open file!" << endl; return; }
		string query_doc;
		getline(read, query_doc); //All queryClear in one line
		read.close();
		//Calculate TF*IDF
		vector<vector<float>> tableScore; //As matrix to ranking
		int i, s_docs = (int)id_docs.size() - 1;
		string rad; RadixNode* end_word; float _idf;
		//To print al TF-IDF per doc
		int pos_id_docs, last, temp, s_labels; string word;
		vector<float> raw;
		vector<string> words_query; bool run = 1;
		while (run) { //for preprocessed words
			if (query_doc.find(' ') == std::string::npos) { word = query_doc; run = 0; }
			else word = query_doc.substr(0, query_doc.find(' '));
			query_doc = query_doc.substr(query_doc.find(' ') + 1);
			end_word = root;
			words_query.push_back(word);
			if (!findWord(word, end_word, rad)) { //If the word didn't find in the tree, print all docs with 0 (TF-IDF)
				for (i = 0; i <= s_docs; i++)
					raw.push_back(0);
				tableScore.push_back(raw);
				raw.clear();
				continue;
			}
			//If the word is found, calculate IDF & print TF-IDFs
			_idf = float(log10(float(s_docs + 1) / float(end_word->labels.size())));
			//TF-IDF per doc in labels_vector from end_word RadixNode 
			//In this part, calculate number of documents between each pair of documents from labels_vector
			//and save documents where are not in labels_vector, save 0.0 (TF-IDF)
			//means, number of documents between, save 0.0 (TF-IDF)
			last = -1, s_labels = end_word->labels.size();
			for (i = 0; i < s_labels; i++) {
				pos_id_docs = temp = findDoc(end_word->labels[i].first) - 1;
				while (pos_id_docs > last) {
					raw.push_back(0);
					pos_id_docs--;
				}
				raw.push_back(float(end_word->labels[i].second) * _idf);
				last = temp + 1;
			}
			//If the last label is or not the final document from id_docs_vector, verify as the above mentioned and save
			last = temp + 1; temp = s_docs;
			while (temp > last) {
				raw.push_back(0);
				temp--;
			}
			tableScore.push_back(raw);
			raw.clear();
		}
		vector<pair<int, float>> info_ranking;
		int count, s_words = (int)words_query.size(); float total;
		for (i = 0; i <= s_docs; i++) { //Verify columns, save the sum of TF-idf(s) and count words per document of id_docs_vector
			count = total = 0;
			for (int j = 0; j < s_words; j++) {
				if (tableScore[j][i] != 0) {
					total += tableScore[j][i];
					count++;
				}
			}
			info_ranking.push_back(pair<int, float>(count, total));
		} //Print the descendent ranking from the doc with have more words of the query and sum the maior result of TF-idf(s), to the less doc_result 
		int s = (int)info_ranking.size(), pos_max, times = s;
		while (times > 0) {
			pos_max = 0;
			for (i = 0; i < s; i++) {
				if (info_ranking[i].first >= info_ranking[pos_max].first && info_ranking[i].second >= info_ranking[pos_max].second)
					pos_max = i;
			}
			if (info_ranking[pos_max].first != 0) {
				cout << "id_doc: " << id_docs[pos_max] << endl;
				for (int j = 0; j < s_words; j++) {
					if (tableScore[j][pos_max] != 0) {
						//print word and TF-idf from the document
						cout << "\"" << words_query[j] << "\": " << tableScore[j][pos_max] << endl;
					}
				}
				cout << endl << endl;
			}
			info_ranking[pos_max].first = info_ranking[pos_max].second = -1;
			times--;
		}
	}
	void rankingDocStr(string query) { //Similarity per doc (for all docs inserted in the tree) //With preprocessing query_string
		//FIRST: preprocess query doc
		vector<string> words_query;
		preprocessing(query, words_query);
		//SECOND: calculate TF*IDF
		vector<vector<float>> tableScore; //As matrix to ranking

		 //for preprocessing words
		int i, s_docs = (int)id_docs.size() - 1;
		string rad, word; RadixNode* end_word; float _idf;
		//To print al TF-IDF per doc
		int pos_id_docs, last, temp, s_labels, i_query = 0, s_query = (int)words_query.size();
		vector<float> raw;
		for (; i_query < s_query; i_query++) {
			word = words_query[i_query];
			end_word = root;
			if (!findWord(word, end_word, rad)) { //If the word didn't find in the tree, print all docs with 0 (TF-IDF)
				for (i = 0; i <= s_docs; i++)
					raw.push_back(0);
				tableScore.push_back(raw);
				raw.clear();
				continue;
			}
			//If the word is found, calculate IDF & print TF-IDFs
			_idf = float(log10(float(s_docs + 1) / float(end_word->labels.size())));
			//TF-IDF per doc in labels_vector from end_word RadixNode 
			//In this part, calculate number of documents between each pair of documents from labels_vector
			//and save documents where are not in labels_vector, save 0.0 (TF-IDF)
			//means, number of documents between, save 0.0 (TF-IDF)
			last = -1, s_labels = end_word->labels.size();
			for (i = 0; i < s_labels; i++) {
				pos_id_docs = temp = findDoc(end_word->labels[i].first) - 1;
				while (pos_id_docs > last) {
					raw.push_back(0);
					pos_id_docs--;
				}
				raw.push_back(float(end_word->labels[i].second) * _idf);
				last = temp + 1;
			}
			//If the last label is or not the final document from id_docs_vector, verify as the above mentioned and save
			last = temp + 1; temp = s_docs;
			while (temp > last) {
				raw.push_back(0);
				temp--;
			}
			tableScore.push_back(raw);
			raw.clear();
		}
		vector<pair<int, float>> info_ranking;
		int count; float total;
		for (i = 0; i <= s_docs; i++) { //Verify columns, save the sum of TF-idf(s) and count words
			count = total = 0;
			for (int j = 0; j < s_query; j++) {
				if (tableScore[j][i] != 0) {
					total += tableScore[j][i];
					count++;
				}
			}
			info_ranking.push_back(pair<int, float>(count, total));
		} //Print the descendent ranking from the doc with have more words of the query and sum the maior result of TF-idf(s), to the less doc_result 
		//Consider both: number of equal words from the query & sum of TF-idf(s) of the words
		int s = (int)info_ranking.size(), pos_max, times = s;
		while (times > 0) {
			pos_max = 0;
			for (i = 0; i < s; i++) {
				if (info_ranking[i].first >= info_ranking[pos_max].first && info_ranking[i].second >= info_ranking[pos_max].second)
					pos_max = i;
			}
			if (info_ranking[pos_max].first != 0) {
				cout << "id_doc: " << id_docs[pos_max] << endl;
				for (int j = 0; j < s_query; j++) {
					if (tableScore[j][pos_max] != 0) { //print word and TF-idf from the document
						cout << "\"" << words_query[j] << "\": " << tableScore[j][pos_max] << endl;
					}
				}
				cout << endl << endl;
			}
			info_ranking[pos_max].first = info_ranking[pos_max].second = -1;
			times--;
		}
	}
	void interfaceScoreRankingStr() { //Interface to score documents where is the word or set of words
		string query_str; //query_str to be score
		int find, option, i, j, s_q, s_t, start, end, s_docs_label, s_second;
		string _word, rad, line_doc, temp_doc, id_doc; RadixNode* end_word;
		float _idf;
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); //Colorizing text in the console //To colorize query_word from doc
		while (1) {
			cout << "----------------- Welcome to interface query_word ranking! -------------" << endl;
			cout << "1. Insert query_str" << endl;
			cout << "2. Exit" << endl;
			cout << "Please insert option: "; cin >> option; cout << endl;
			switch (option) {
			case 1: {
				cout << "You choose option 1! So ..." << endl;
				cout << "Query_str is: ";
				getline(cin, query_str); getline(cin, query_str);
				vector<string> clear;
				preprocessing(query_str, clear); //clear words
				int w = 0, s_w = (int)clear.size();
				//TO PRINT: Print doc_id |+| text with the word (depends in how many times is repeated) |+| score TF*IDF
				cout << endl << "docs_id	||	Text with the word	||	TF-IDF" << endl << endl;
				//Note: Print text where is the word, word by word, not print two or more words in a text, means: print first all documents where content word A,
				//then print all documents where content word B, and consecutively do this
				for (; w < s_w; w++) { //Score one by one word from query_str using clear_vector
					_word = clear[w]; s_q = (int)clear[w].size(); end_word = root;
					//FIRST: Verify if the word exists in the tree
					if (!findWord(_word, end_word, rad)) { //If the word didn't find in the tree
						cout << "This word doesn't exist in the tree. Try again, please." << endl; break;
					}
					//SECOND: Calculate TF*IDF of all docs where is the word
					_idf = float(log10(float((float)id_docs.size()) / float(end_word->labels.size())));
					//TF-IDF per doc 
					vector<pair<string, int>> labels_copy = end_word->labels;
					int s = (int)labels_copy.size(), pos_max = 0, times = s;
					while (times > 0) { //Descendent ranking from the maior number of times is the word repeated to the less
						pos_max = 0;
						for (i = 0; i < s; i++) {
							if (labels_copy[i].second > labels_copy[pos_max].second)
								pos_max = i;
						}
						labels_copy[pos_max].first = ""; labels_copy[pos_max].second = -1;
						times--;

						ifstream read("documents.txt"); //Read fron txt without preprocessing to print later where is the word
						id_doc = end_word->labels[pos_max].first;
						while (getline(read, line_doc)) {
							find = line_doc.find(id_doc);
							if (find != std::string::npos) break; //find doc in file
						}
						read.close();
						line_doc = line_doc.substr(line_doc.find(' ') + 1); //Remove id_doc, get only abstract
						temp_doc = line_doc; //string equal to doc
						s_t = int(temp_doc.size());
						transform(temp_doc.begin(), temp_doc.end(), temp_doc.begin(), ::tolower); //string tolower to find word
						//Print information doc depends in how many times the word is repeated
						j = 0;
						s_second = end_word->labels[pos_max].second;
						while (j < s_second) { //.second means how many times the word is repeated in this doc
							find = temp_doc.find(clear[w]); //find pos of query_word in doc
							start = find - 1;
							end = find + s_q;
							//Verify if the word is not a substring && case of (\n)WORD && case of first/last word of temp_doc, print text with word
							if ((start == -1 || (start >= 0 && (!isalpha(temp_doc[start]) || (temp_doc[start] == 'n' && start - 1 >= 0 && temp_doc[start - 1] == '\\')))) && (end == s_t || (end < s_t && !isalpha(temp_doc[end])))) {
								//This word is like this: [#1_/"?..]word[#1_/"?..], where [#1_/"?..] means not-alphabetic, include (\n)WORD case 
								cout << id_doc << " || "; // id_doc
								cout << line_doc.substr(0, find); // text until the word //print from line_doc
								SetConsoleTextAttribute(h, 47); //text with color white and highlight green
								cout << line_doc.substr(find, s_q); // the word //print from line_doc
								SetConsoleTextAttribute(h, 15); //text with color white
								cout << " || " << float(s_second) * _idf << endl; // score TF*IDF
								j++; //continue with the next occurrence of word
							} //Reduce strings_doc each time that word is found (maybe the word is a substring or not)
							temp_doc = temp_doc.substr(end); //save substr temp_doc
							line_doc = line_doc.substr(end); //save substr line_doc
						}
					}
				}
				cout << endl; break;
			}
			case 2:
				cout << "You choose option 2! So ... Bye!" << endl; return; break;
			default:
				cout << "Error! Try again, please." << endl; break;
			}
			system("PAUSE"); system("cls");
		}
	}
};
void menu() {
	int option, numb_docs, n; //number of docs to be inserted
	string query_doc; //name of query_document OR query_string to be scored
	string document; //string to reuse for the file to read
	chrono::time_point<chrono::high_resolution_clock> start, end; //To measure time
	int64_t duration; //To measure time
	while (1) {
		cout << "					Welcome to final project! " << endl;
		cout << "1. Table_score doc without preprocessing" << endl; //table_score .csv of A doc without preprocessing dataset
		cout << "2. Table_score doc with preprocessing" << endl; //table_score .csv of A doc with preprocessing dataset
		cout << "3. Ranking doc" << endl; //print descendent ranking doc 
		cout << "4. Ranking str" << endl; //print descendent ranking str 
		cout << "5. Interface" << endl; //interface, print where is word by word in docs
		cout << "6. Exit" << endl;
		cout << "Please insert option: "; cin >> option; cout << endl;
		switch (option) {
		case 1: {
			cout << "You choose option 1! So ..." << endl << endl;
			cout << "Please, insert number of documents: "; cin >> numb_docs;
			cout << "Please, insert name of query_doc: "; cin >> query_doc;
			cout << endl << endl;
			//Insert docs to tree (numb_docs) & to measure time
			RadixTree tree; //create new structure to have data consistence
			n = numb_docs;
			ifstream read("documentsClear.txt"); //document with the id of documents and clear data (data from the .json)
			if (read.fail()) { cout << "ERROR: Don't open file!" << endl; return; }
			start = chrono::high_resolution_clock::now();
			while (n > 0) {
				getline(read, document);
				tree.insertDoc(document); //INSERT DOC in the tree
				//cout << n << endl;
				n--;
			}
			end = chrono::high_resolution_clock::now();
			duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
			cout << "Insert " << numb_docs << " doc(s) - Duration in milliseconds: " << duration << endl;
			//Score query_doc & continue measure time
			start = chrono::high_resolution_clock::now();
			tree.scoreDoc(query_doc); //SCORE
			end = chrono::high_resolution_clock::now();
			duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
			cout << "Table_score for " << numb_docs << " doc(s) - Duration in milliseconds: " << duration << endl << endl;
			break;
		}
		case 2: {
			cout << "You choose option 2! So ..." << endl << endl;
			cout << "Please, insert number of documents: "; cin >> numb_docs;
			cout << "Please, insert name of query_doc: "; cin >> query_doc;
			cout << endl << endl;
			//Insert docs to tree (numb_docs) & to measure time
			RadixTree tree; //create new structure to have data consistence
			n = numb_docs;
			ifstream read("documents.txt"); //document with the id of documents and clear data (data from the .json)
			if (read.fail()) { cout << "ERROR: Don't open file!" << endl; return; }
			start = chrono::high_resolution_clock::now();
			while (n > 0) {
				getline(read, document);
				tree.insertDocWthPre(document); //INSERT DOC in the tree
				//cout << n << endl;
				n--;
			}
			end = chrono::high_resolution_clock::now();
			duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
			cout << "Insert " << numb_docs << " doc(s) - Duration in milliseconds: " << duration << endl;
			//Score query_doc & continue measure time
			start = chrono::high_resolution_clock::now();
			tree.scoreDocWthPre(query_doc); //SCORE
			end = chrono::high_resolution_clock::now();
			duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
			cout << "Table_score for " << numb_docs << " doc(s) - Duration in milliseconds: " << duration << endl << endl;
			break;
		}
		case 3: {
			cout << "You choose option 3! So ..." << endl << endl;
			cout << "Please, insert number of documents: "; cin >> numb_docs;
			cout << "Please, insert name of query_doc: "; cin >> query_doc;
			cout << endl << endl;
			//Insert docs to tree (numb_docs) & to measure time
			RadixTree tree; //create new structure to have data consistence
			n = numb_docs;
			ifstream read("documentsClear.txt"); //document with the id of documents and clear data (data from the .json)
			if (read.fail()) { cout << "ERROR: Don't open file!" << endl; return; }
			start = chrono::high_resolution_clock::now();
			while (n > 0) {
				getline(read, document);
				tree.insertDoc(document); //INSERT DOC in the tree
				//cout << n << endl;
				n--;
			}
			end = chrono::high_resolution_clock::now();
			duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
			cout << "Insert " << numb_docs << " doc(s) - Duration in milliseconds: " << duration << endl;
			//Score rankingDocFile & continue measure time
			start = chrono::high_resolution_clock::now();
			tree.rankingDocFile(query_doc); //SCORE
			end = chrono::high_resolution_clock::now();
			duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
			cout << "Ranking " << numb_docs << " doc(s) - Duration in milliseconds: " << duration << endl << endl;
			break;
		}
		case 4: {
			cout << "You choose option 4! So ..." << endl << endl;
			cout << "Please, insert number of documents: "; cin >> numb_docs;
			cout << "Please, insert query: ";
			getline(cin, query_doc); getline(cin, query_doc);
			cout << endl << endl;
			//Insert docs to tree (numb_docs) & to measure time
			RadixTree tree; //create new structure to have data consistence
			n = numb_docs;
			ifstream read("documentsClear.txt"); //document with the id of documents and clear data (data from the .json)
			if (read.fail()) { cout << "ERROR: Don't open file!" << endl; return; }
			start = chrono::high_resolution_clock::now();
			while (n > 0) {
				getline(read, document);
				tree.insertDoc(document); //INSERT DOC in the tree
				//cout << n << endl;
				n--;
			}
			end = chrono::high_resolution_clock::now();
			duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
			cout << "Insert " << numb_docs << " doc(s) - Duration in milliseconds: " << duration << endl << endl;
			//Score rankingDocStr & continue measure time
			start = chrono::high_resolution_clock::now();
			tree.rankingDocStr(query_doc); //SCORE
			end = chrono::high_resolution_clock::now();
			duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
			cout << "Ranking " << numb_docs << " doc(s) - Duration in milliseconds: " << duration << endl << endl;
			break;
		}
		case 5: {
			cout << "You choose option 5! So ..." << endl << endl;
			cout << "Please insert number of documents: "; cin >> numb_docs;
			cout << endl;
			//Insert docs to tree (numb_docs)
			RadixTree tree; //create new structure to have data consistence
			ifstream read("documentsClear.txt"); //document with the names of documents (from .json)
			while (numb_docs > 0) {
				getline(read, document);
				tree.insertDoc(document);
				numb_docs--;
			}
			read.close();
			tree.interfaceScoreRankingStr(); //call to interface function
			cout << endl;
			break;
		}
		case 6:
			cout << "You choose option 6! So ... Bye!" << endl;
			return; break;
		default:
			cout << "Error! Try again, please." << endl; break;
		}
		system("PAUSE"); system("cls");
	}
}
void main() {
	//readJSON(); //To reduce original data JSON
	menu();
}