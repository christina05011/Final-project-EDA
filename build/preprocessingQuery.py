import pandas as pd
import re
import string
import nltk
from nltk.tokenize import word_tokenize
from nltk.corpus import stopwords

file = open("query.txt","r") #open file with [id] abstract
line = file.read()
print(line)
out_file = open("queryClear.txt","w") #write file with [id] abstract_clear

#line = line.replace('\\n',' ') #Change \n for space
line = re.sub('-',' ',line) #Change - for space
line = re.sub('_',' ',line) #Change _ for space
line = re.sub(r'[^\w\s]','',line.lower()) #Remove punctuation
line = re.sub(r'[0-9]','',line) #Remmove numbers
sw=stopwords.words('english')
line = [word for word in word_tokenize(line) if not word in sw] #Remove stopwords
#print(line)
out_file.write(' '.join(line) + '\n')
line=file.readline()
    
file.close()
out_file.close()
