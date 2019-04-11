#include <iostream>
#include<fstream>
#include<cstdio>
#include <algorithm>
#include <vector>
#include <sstream>
#include <string>
#include <cmath>
#include <map>

using namespace std;

class Record {
public:
    char nombre[30];
    char carrera[20];
    int ciclo;
    int nextDel;

    Record(string nom, string carr, int cic,int ndel = -2)
    {
        stringstream sstream;
        sstream << nom;
        sstream >> nombre;

        stringstream sstream2;
        sstream2 << carr;
        sstream2 >> carrera;
        sstream.str("");

        ciclo = cic;

        nextDel = ndel;
    }

    Record(){}

    void operator=(Record obj)
    {
        string nom(obj.nombre);
        string carr(obj.carrera);
        int cic = obj.ciclo;
        int ndel = obj.nextDel;

        stringstream sstream;
        sstream << nom;
        sstream >> nombre;

        stringstream sstream2;
        sstream2 << carr;
        sstream2 >> carrera;
        sstream.str("");

        ciclo = cic;

        nextDel = ndel;

        
        return;
    }

    void setData() {
        cout << "Alumno:";
        cin >> nombre;
        cout << "Carrera: ";
        cin >> carrera;
        cout << "Ciclo: ";
        cin >> ciclo;
        nextDel = -2;
    }

    void showData() {
        cout << "Nombre: " << nombre<<endl;
        cout << "Carrera: " << carrera<<endl;
        cout << "Ciclo : " << ciclo<<endl;

    }

    int getNextDel() { return nextDel; }

    void setNextDel(int _nextDel) { this->nextDel = _nextDel; }
};

class IndexedSequentialFile
{
private:
    string fileName;
    string indexName;

    map<string,int> index;

public:
    IndexedSequentialFile(string fileName, string indexName = "index2.dat") {
        this->fileName = fileName;
        this->indexName = indexName;
        initFreeList();
        loadIndex();
    }

    void writeRecord(Record obj) {
        ofstream outFile;
        outFile.open(fileName, ios::binary | ios::app);
        outFile.write((char *) &obj, sizeof(obj));
        outFile.close();
    }

    void scanAll() {
        ifstream inFile;
        inFile.open(fileName, ios::binary);
        
        int delPointer;
        inFile.read((char *) &delPointer, sizeof(int));
        
        Record obj;
        while (inFile.read((char *) &obj, sizeof(obj))) {
        if (obj.getNextDel() == -2)
           {
                obj.showData();
           }
        }
        inFile.close();
    }


    void deleteRecord(int n) {
        fstream inFile;
        Record obj;
        inFile.open(this->fileName, ios::in | ios::out | ios::binary);
        if (inFile.is_open()) {
            int delPointer;
            inFile.read((char *) &delPointer, sizeof(int));
            //read the record
            inFile.seekg(n * sizeof(Record) + sizeof(int));
            inFile.read((char*) &obj, sizeof(Record));
            //change its delete pointer
            obj.setNextDel(delPointer);
            //write the record
            inFile.seekg(n * sizeof(Record) + sizeof(int));
            inFile.write((char*) &obj, sizeof(Record));
            //write the delete pointer
            inFile.seekg(0, ios::beg);
            inFile.write((char *) &n, sizeof(int));

            inFile.close();

            string key(obj.nombre);
            index.erase(key);

        }else cout << "Could not open the file.\n";
    }

    void insertRecord(Record  obj) {
        Record temp;
        fstream outFile;
        outFile.open(this->fileName, ios::in | ios::out | ios::binary);
        if (outFile.is_open()) {
            int delPointer;
            outFile.read((char *) &delPointer, sizeof(int));

            if(delPointer == -1){
                outFile.seekg(0, ios::end);
                
                int address = outFile.tellg();
                string key(obj.nombre);

                outFile.write((char *) &obj, sizeof(obj));

                index[key] = address;

            } 

            else {
                //read the current record
                outFile.seekg(delPointer * sizeof(Record) + sizeof(int));
                outFile.read((char *) &temp, sizeof(temp));
                //write the new record
                outFile.seekg(delPointer * sizeof(Record) + sizeof(int));
                outFile.write((char *) &obj, sizeof(obj));
                //change the del pointer
                delPointer = temp.getNextDel();
                //write the delete pointer
                outFile.seekg(0, ios::beg);
                outFile.write((char *) &delPointer, sizeof(int));
                
                int address = delPointer * sizeof(Record);
                string key(obj.nombre);
                index[key] = address;
            }
            outFile.close();
        }else cout << "Could not open the file.\n";
    }

    void initFreeList() {
        bool ready = false;
        fstream inFile;
        //check if the free-list has already been created
        inFile.open(this->fileName, ios::in | ios::binary | ios::ate);
        if (inFile.is_open()) {
            long bytes = inFile.tellg();
            ready = bytes >= (sizeof(Record) + sizeof(int));
            inFile.close();
        }
        //create the file with the delete pointer to null
        if (!ready) {
            remove(this->fileName.c_str());
            inFile.open(this->fileName, ios::out | ios::binary);
            int firstPointer = -1;
            inFile.write((char *) &firstPointer, sizeof(int));
            inFile.close();
        }
    }

    int size() {
        int numRecords = 0;
        fstream inFile;
        inFile.open(this->fileName, ios::in | ios::binary);
        if (inFile.is_open()) {
            inFile.seekg(0, ios::end);
            long bytes = inFile.tellg();
            numRecords = (bytes - sizeof(int)) / sizeof(Record);
            inFile.close();
        } else cout << "Could not open the file.\n";
        return numRecords;
    }

    //create index from data: deletes all contents previously in index map 
    void makeIndex()
    {
        index.clear();
        ifstream inFile;
        inFile.open(fileName, ios::binary);
        int address = 0;        
        int delPointer;
        inFile.read((char *) &delPointer, sizeof(int));
        address += sizeof(int);
        Record obj;
        while (inFile.read((char *) &obj, sizeof(obj))) {
        if (obj.getNextDel() == -2)
           { 
            string key(obj.nombre);
            index[key] = address;
           }
           address += sizeof(obj);
        }
        inFile.close();

        return;
    }


    // writes index map in index file
    void writeIndex()
    {  
        ofstream file;
        file.open(indexName, ofstream::out | ofstream::trunc);
        file.close();

        fstream outFile;
        outFile.open(indexName,ios::out | ios::binary | ios::app);
        if(outFile.is_open())
        {
            map<string,int>::iterator it;
            
            char key[30];
            for(it = index.begin(); it != index.end(); it++)
            {
                stringstream sstream;
                sstream << it->first;
                sstream >> key;
                int address = it->second;
                outFile.write(key,sizeof(key)/sizeof(char));
                outFile.write((char *) &address,sizeof(int));
            }

            outFile.close();
        }
        return;
    }

    //loads index map from index file
    void loadIndex()
    {
        fstream inFile;
        inFile.open(fileName,ios::in | ios::binary);
        if(inFile.is_open())
        {
            while(!inFile.eof())
            {
                char key[30];
                int address;
                inFile.read(key, sizeof(key)/sizeof(char));   
                inFile.read((char *) &address, sizeof(address));
                string nameKey(key);
                index[nameKey] = address;
            }
            inFile.close();
        }
        return;
    }

    Record search(string key)
    {
        fstream inFile;
        inFile.open(fileName, ios::in | ios::binary);
        inFile.seekg (index[key],ios::beg);
        Record obj;
        inFile.read((char *) &obj, sizeof(obj));
        return obj;
    }

    void reset()
    {
        ofstream file;
        file.open(fileName, ofstream::out | ofstream::trunc);
        file.close();   
        initFreeList();

        file.open(indexName, ofstream::out | ofstream::trunc);
        file.close();   
        index.clear();
    }
    

};


int main()
{
    Record alu1("Rodrigo2","CS",8);
    Record alu2("Bryan","CS",9);
    Record alu3("ElBryan","CS",10);
    Record alu4("Alumno","CS",8);
    Record alu5("Student","CS",9);
    Record alu6("Rodrigo","CS",10);
    Record alu7("Rodrigo2","CS",8);
    Record alu8("Bryan","CS",9);
    Record alu9("ElBryan","CS",10);
    Record alu10("Alumno","CS",8);
    Record alu11("Student","CS",9);
    Record alu12("Rodrigo","CS",10);
    Record alu13("Rodrigo","CS",10);
    Record alu14("Rodrigo","CS",10);
    Record alu15("Rodrigo","CS",10);
    Record alu16("Alumno","CS",8);
    Record alu17("Student","CS",9);
    Record alu18("Rodrigo","CS",10);
    Record alu19("Rodrigo","CS",10);
    Record alu20("Rodrigo","CS",10);
    

    vector<Record> alumnos;
    alumnos.push_back(alu1);
    alumnos.push_back(alu2);
    alumnos.push_back(alu3);
    alumnos.push_back(alu4);
    alumnos.push_back(alu5);
    alumnos.push_back(alu6);
    alumnos.push_back(alu7);
    alumnos.push_back(alu8);
    alumnos.push_back(alu9);
    alumnos.push_back(alu10);
    alumnos.push_back(alu11);
    alumnos.push_back(alu12);
    alumnos.push_back(alu13);
    alumnos.push_back(alu14);
    alumnos.push_back(alu15);
    alumnos.push_back(alu16);
    alumnos.push_back(alu17);
    alumnos.push_back(alu18);
    alumnos.push_back(alu19);
    alumnos.push_back(alu20);
    

    IndexedSequentialFile file("datos2.dat");
    file.reset();

    for(int i = 0; i < alumnos.size(); i++)
    {
        file.insertRecord(alumnos[i]);
    }

    file.writeIndex();

    file.search("Bryan").showData();
    return 0;
}

