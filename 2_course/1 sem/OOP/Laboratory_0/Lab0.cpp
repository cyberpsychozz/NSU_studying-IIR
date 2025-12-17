#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <vector>

using namespace std;

vector<string> parser(const string& line)
{
    vector<string> parsed_line;
    string word;
    bool fwidx = true;
    for (char c : line)
    {
        if ((isdigit(c) || isalpha(c)) && !isspace(c)){
            word.push_back(c);
            fwidx = false;
        }
        else 
        {
            if (!fwidx && !word.empty())
            {
                parsed_line.push_back(word);
                word.clear();
            }    
        }
    }
    
    if(!fwidx && !word.empty()){
        parsed_line.push_back(word);
        word.clear();
    }
    
    return parsed_line;
}

int main(){
    //file opening
    ifstream inFile("input.txt");
    ofstream outFile("data.csv");

    if(!inFile)
    {
        cerr << "opening input file error!" << endl;
        return 1;
    }

    if(!outFile)
    {
        cerr << "opening output file error!" << endl;
        return 1;
    }

    outFile << "Word|Frequency|Frequency(%)" << endl;
    outFile << "---------------------------" << endl;

    string line;
    vector<string> parsed_line;

    map<string, int> temp;
    int ammount = 0;

    while(getline(inFile, line))
    {
        parsed_line = parser(line);
        for (string  word : parsed_line) 
        {
            temp[word]++;
            ammount++;

        }
    }


    
    for (const auto& [word, count] : temp)
    {
        outFile << word << "|" << count << "|" << ((float)count ) / ammount * 100 << "%" << endl;
    }

    outFile.close();
    inFile.close();
    return 0;
}