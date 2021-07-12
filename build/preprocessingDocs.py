import pandas as pd
import re
import nltk
from nltk.tokenize import word_tokenize
from nltk.corpus import stopwords

file = open("documents.txt","r") #open file with [id] abstract
line = file.readline()
out_file = open("documentsClear.txt","w") #write file with [id] abstract_clear
i = 1;
while line:
    _id = line[0:line.find(' ') + 1] #Save id 
    out_file.write(_id)
    line=line[line.find(' '):]
    #print(line)
    line = line.replace('\\n',' ') #Change \n for space
    line = re.sub('-',' ',line) #Change - for space
    line = re.sub('_',' ',line) #Change _ for space
    line = re.sub(r'[^\w\s]','',line.lower()) #Remove punctuation
    line = re.sub(r'[0-9]','',line) #Remmove numbers
    sw=stopwords.words('english')
    line = [word for word in word_tokenize(line) if not word in sw] #Remove stopwords
    #print(line)
    out_file.write(' '.join(line) + '\n')
    print(i , '\n')
    i+=1
    line = file.readline()
    
file.close()
out_file.close()
