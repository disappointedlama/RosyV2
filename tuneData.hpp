#pragma once
#include<fstream>
#include"position.cpp"
#include"position.hpp"
void generateTuneData(){
    std::vector<std::pair<std::string,int>> values{};
    std::ifstream input{"tuneData/in.txt"};
    int writes{0};
    if(input.is_open()){
        std::string line, fen;
        int score;
        while(std::getline(input,line)){
            if(values.size()%1000==0) std::cout<<values.size()<<std::endl;
            if(values.size()%100000==0&&values.size()){
                std::ofstream tmpOut{"tuneData/tmp"+std::to_string(writes++)+".txt"};
                if(tmpOut.is_open()){
                    for(const auto& p: values){
                        tmpOut<<p.first<<" "<<p.second<<"\n";
                    }
                    tmpOut.close();
                }
                values=std::vector<std::pair<std::string,int>>{};
            }
            size_t pos{line.find(" ")};
            fen=line.substr(0,pos);
            score=std::stoi(line.substr(pos+1));
            bool alreadyIncluded{false};
            for(const auto& p: values){
                if(p.first==fen){
                    alreadyIncluded=true;
                    break;
                }
            }
            if(!alreadyIncluded){
                values.push_back(std::make_pair(fen,score));
            }
        }
        input.close();
    }
    std::ofstream tmpOut{"tuneData/tmp"+std::to_string(writes++)+".txt"};
    if(tmpOut.is_open()){
        for(const auto& p: values){
            tmpOut<<p.first<<" "<<p.second<<"\n";
        }
        tmpOut.close();
    }
    values=std::vector<std::pair<std::string,int>>{};
    for(int i=0;i<writes;++i){    
        std::ifstream tempFile{"tuneData/tmp"+std::to_string(i)+".txt"};
        int writes{0};
        if(tempFile.is_open()){
            std::string line, fen;
            int score;
            while(std::getline(tempFile,line)){
                if(values.size()%1000==0) std::cout<<values.size()<<std::endl;
                size_t pos{line.find(" ")};
                fen=line.substr(0,pos);
                score=std::stoi(line.substr(pos+1));
                bool alreadyIncluded{false};
                for(const auto& p: values){
                    if(p.first==fen){
                        alreadyIncluded=true;
                        break;
                    }
                }
                if(!alreadyIncluded){
                    values.push_back(std::make_pair(fen,score));
                }
            }
            tempFile.close();
        }
    }
    std::ofstream output{"tuneData/out.txt"};
    if(output.is_open()){
        for(const auto& p: values){
            output<<p.first<<" "<<p.second<<"\n";
        }
        output.close();
    }
}