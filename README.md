# Final-project-EDA
This repository has:
1. Python code to preprocess documents from dataset.
2. Python code to preprocess query documents.
3. CMakeLists.txt
4. A pdf with time tables.
5. Main code.

To run this code is necessary to download documents from:
https://drive.google.com/drive/folders/1GyTJs0WuUFWIh7CDhELWKohfRXn02Jq4?usp=sharing

In which are:
- documents.txt: Dataset with only id and abstract extracted from dataset .json 
	(downloaded from https://www.kaggle.com/Cornell-University/arxiv)
- documentsClear.txt: Preprocessed documents.txt by python code previously mentioned.
- stopwordsEnglish.txt: Document, with all stopwords of English language, to be opened by preprocessing function in C++.
- query.txt: Abstract, from Retroactive Data Structures paper, that is a query document. Example of query document, could be other.
- queryClear.txt: Preprocessed query.txt by python code previously mentioned.

Note: All datasets are this form: [id] abstract. Except query document.

Also, to run this code is necessary to paste main.cpp (main code) in a C++ project.
And run this C++ code with all documents downloaded. 

Note: It is not necessary paste CMakeLists.txt.
And python codes were not used in C++ code. They were only to help in the dataset preprocessing.

Author: Christina Chacón
