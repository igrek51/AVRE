#include <iostream>
#include <fstream>
#include <conio.h>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace std;

string version = "1.4.2";

void error(string e){
	cout<<"!!! ERROR: "<<e<<endl;
}

void ok(){
	cout<<"-- OK --"<<endl<<endl;
}

void ss_clear(stringstream &sstream){
	sstream.str("");
	sstream.clear();
}

int hex_to_int(char hex){
	if(hex>='0'&&hex<='9') return hex-'0';
	if(hex>='A'&&hex<='F') return hex-'A'+10;
	return 0;
}

int ihex_size(string filename){
	//wczytanie pliku
	fstream plik;
	plik.open(filename.c_str(),fstream::in|fstream::binary);
	if(!plik.good()){
		error("brak pliku "+filename);
		plik.close();
		return -1;
	}
	plik.seekg(0,plik.end);
	unsigned int fsize = plik.tellg();
	char *file_tab = new char [fsize];
	plik.seekg(0,plik.beg);
	plik.read(file_tab,fsize);
	plik.close();
	//analiza pliku
	int bajty=0, bajty_2=0;
	int colons=0;
	char znak;
	for(int i=0; i<fsize; i++){
		znak=file_tab[i];
		if(znak==':'){
			colons++;
			if(i<fsize-2){
				bajty+=hex_to_int(file_tab[i+1])*16;
				bajty+=hex_to_int(file_tab[i+2]);
			}
		}
		if((znak>='0'&&znak<='9')||(znak>='A'&&znak<='F')) bajty_2++;
	}
	delete[] file_tab;
	if(bajty_2%2==1){
		stringstream ss;
		ss<<"nieparzysta liczba znakow ("<<bajty_2<<")";
		error(ss.str());
		return -1;
	}
	if(bajty!=bajty_2/2-colons*5){
		error("blad analizy bajtow");
		return -1;
	}
	return bajty;
}

int file_size(string filename){
	fstream plik;
	plik.open(filename.c_str(),fstream::in|fstream::binary);
	if(!plik.good()){
		cout<<"Error: brak pliku"<<endl;
		plik.close();
		return 0;
	}
	plik.seekg(0,plik.end);
	int fsize = plik.tellg();
	plik.close();
	return fsize;
}

char *get_file(string filename, int &fsize){
	fstream plik;
	plik.open(filename.c_str(),fstream::in|fstream::binary);
	if(!plik.good()){
		cout<<"Error: brak pliku"<<endl;
		plik.close();
		return NULL;
	}
	plik.seekg(0,plik.end);
	fsize = plik.tellg();
	char *file_tab = new char [fsize];
	plik.seekg(0,plik.beg);
	plik.read(file_tab,fsize);
	plik.close();
	return file_tab;
}

void show_output(){
	int fsize=0;
	char *file_tab = get_file("output.txt",fsize);
	if(file_tab==NULL) return;
	for(int i=0; i<fsize; i++){
		cout<<file_tab[i];
	}
	delete[] file_tab;
}

bool file_exists(string filename){
	fstream plik;
	plik.open(filename.c_str(),fstream::in|fstream::binary);
	if(plik.good()){
		plik.close();
		return true;
	}else{
		plik.close();
		return false;
	}
}

string* load_config(int config_num){
  if(!file_exists("avre_config.ini")) return NULL;
	fstream plik;
	plik.open("avre_config.ini",fstream::in|fstream::binary);
	if(!plik.good()){
		plik.close();
		return NULL;
	}
	string *config = new string [config_num];
	for(int i=0; i<config_num; i++) config[i] = "";
	string linia, linia2;
	for(int nrl=1; !plik.eof(); nrl++){
		getline(plik,linia,'\n');
		if(nrl%2==1) continue; //komentarz w pliku
		if(nrl/2-1>=config_num) break; //zbêdne wiersze w pliku
		linia2="";
		for(unsigned int i=0; i<linia.length(); i++){
			if(linia[i]=='\r'||linia[i]=='\n') continue;
			linia2+=linia[i];
		}
		config[nrl/2-1]=linia2;
	}
	plik.close();
	return config;
}

void show_menu(){
	system("cls");
	cout<<"\t\t\tIgrek AVR Environment v"<<version<<endl;
	cout<<"ESC, q (quit) - wyjscie"<<endl;
	cout<<"t (test) - avrdude + USBasp + ATmega8 test"<<endl;
	cout<<"e (edit) - edytuj kod"<<endl;
	cout<<"d (directory) - otworz folder"<<endl;
	cout<<"c (compile) - kompiluj"<<endl;
	cout<<"s (size) - rozmiar programu w pliku hex"<<endl;
	cout<<"w (write) - wgraj program"<<endl;
	cout<<"1,2,3 - dodatkowe polecenia"<<endl;
	cout<<endl;
}

bool rozmiar_ihex(){
	int rozmiar = ihex_size("flash.hex");
	if(rozmiar<0){
		return false;
	}
	cout<<"Rozmiar pliku HEX: "<<rozmiar<<" B";
	if(rozmiar>1024) cout<<" ("<<(double)rozmiar/1024<<" KB)";
	cout<<endl;
	ok();
	return true;
}

bool kompiluj(string avr_gcc_dir){
	int rozmiar0 = 0;
	if(file_exists("flash.hex")){
		cout<<"Rozmiar poprzedniego pliku HEX: ";
		rozmiar0 = ihex_size("flash.hex");
		if(rozmiar0>0){
			cout<<rozmiar0<<" B";
			if(rozmiar0>1024) cout<<" ("<<(double)rozmiar0/1024<<" KB)";
			cout<<endl<<endl;
		}
	}
	cout<<"Kompilacja..."<<endl;
	//czyszczenie
	/*
	if(file_exists("flash.o")) system("del flash.o");
	if(file_exists("flash.elf")) system("del flash.elf");
	if(file_exists("flash.hex")) system("del flash.hex");
	*/
	if(!file_exists("flash.c")){
		error("Brak pliku flash.c");
		return false;
	}
	stringstream ss;
	//1. krok
	ss<<"\""<<avr_gcc_dir<<"avr-gcc\" -g -Os -mmcu=atmega8 -c flash.c";
	cout<<ss.str().c_str()<<endl;
	if(system(ss.str().c_str())!=0){
		error("Blad kompilacji (1.1)");
		return false;
	}
	if(!file_exists("flash.o")){
		error("Blad kompilacji (1.2)");
		return false;
	}
	//2. krok
	ss_clear(ss);
	ss<<"\""<<avr_gcc_dir<<"avr-gcc\" -g -mmcu=atmega8 -o flash.elf flash.o";
	cout<<ss.str().c_str()<<endl;
	if(system(ss.str().c_str())!=0){
		error("Blad kompilacji (2.1)");
		return false;
	}
	if(!file_exists("flash.elf")){
		error("Blad kompilacji (2.2)");
		return false;
	}
	//3. krok
	ss_clear(ss);
	ss<<"\""<<avr_gcc_dir<<"avr-objcopy\" -j .text -j .data -O ihex flash.elf flash.hex";
	cout<<ss.str().c_str()<<endl;
	if(system(ss.str().c_str())!=0){
		error("Blad kompilacji (3.1)");
		return false;
	}
	if(!file_exists("flash.hex")){
		error("Blad kompilacji (3.2)");
		return false;
	}
	//analiza pliku
	int rozmiar = ihex_size("flash.hex");
	if(rozmiar<=0){
		error("Blad analizy pliku HEX");
		return false;
	}
	cout<<"Rozmiar nowego pliku HEX: "<<rozmiar<<" B";
	if(rozmiar>1024) cout<<" ("<<(double)rozmiar/1024<<" KB)";
	cout<<endl;
	if(rozmiar0>0){
		cout<<endl<<"Roznica rozmiarow: ";
		int roznica = rozmiar-rozmiar0;
		if(roznica>0) cout<<"+";
		cout<<roznica<<" B";
		if(roznica>1024) cout<<" ("<<(double)roznica/1024<<" KB)";
		cout<<endl;
	}
	ok();
	return true;
}

bool usbasp_test(string avr_gcc_dir){
	cout<<"test avrdude + USBasp + ATmega8..."<<endl;
	stringstream ss;
	ss<<"\""<<avr_gcc_dir<<"avrdude.exe\" -p m8 -c usbasp -P usb";
	cout<<ss.str().c_str()<<endl;
	if(system(ss.str().c_str())!=0){
		error("blad USBasp");
		cout<<endl<<"Mozliwe przyczyny bledu:"<<endl;
		cout<<"- Nieprawidlowa sciezka do avrdude"<<endl;
		cout<<"- System Windows nie zostal uruchomiony z wylaczonym wymuszaniem podpisow sterownikow (F8)"<<endl;
		cout<<"- Niezainstalowany lub niewykryty programator USBasp"<<endl;
		cout<<"- Niewykryty mikrokontroler ATmega8 (zle podlaczony lub niestykajacy)"<<endl;
		cout<<"- Nieprawidlowe ustawienie zworek na programatorze"<<endl;
		return false;
	}
	ok();
	return true;
}

bool write_program(string avr_gcc_dir){
	if(!kompiluj(avr_gcc_dir)) return false;
	cout<<"wgrywanie programu..."<<endl;
	stringstream ss;
	ss<<"\""<<avr_gcc_dir<<"avrdude.exe\" -p m8 -c usbasp -P usb -U flash:w:flash.hex";
	cout<<ss.str().c_str()<<endl;
	if(system(ss.str().c_str())!=0){
		error("blad USBasp");
		return false;
	}
	ok();
	return true;
}

bool edit_source(string edytor){
	if(!file_exists("flash.c")){
		error("brak pliku zrodlowego flash.c");
		return false;
	}
	if(edytor.length()==0){
		cout<<"Otwieranie domyslnego edytora..."<<endl;
		if(system("start flash.c")!=0){
			error("blad polecenia system start");
			return false;
		}
	}else{
		cout<<"Otwieranie edytora "<<edytor<<endl;
		stringstream ss;
		ss<<"start \""<<edytor<<"\" flash.c";
		if(system(ss.str().c_str())!=0){
			error("blad polecenia system start");
			return false;
		}
	}
	ok();
	return true;
}

bool open_directory(){
	cout<<"otwieranie folderu..."<<endl;
	if(system("start explorer.exe \".\"")!=0){
		error("blad otwierania folderu");
		return false;
	}
	ok();
	return true;
}

bool cmd_exec(string cmd){
	if(cmd.length()==0){
		error("brak polecenia");
		return false;
	}
	cout<<"Polecenie:"<<endl<<cmd<<endl;
	if(system(cmd.c_str())!=0){
		error("blad polecenia");
		return false;
	}
	ok();
	return true;
}

int main(int argc, char **argv){
	string *config = load_config(5);
	string cmd1="", cmd2="", cmd3="", edytor="", avr_gcc_dir="";
	if(config!=NULL){
		cmd1 = config[0];
		cmd2 = config[1];
		cmd3 = config[2];
		edytor = config[3];
		avr_gcc_dir = config[4];
		if(avr_gcc_dir.length()>0){
			if(avr_gcc_dir[avr_gcc_dir.length()-1]!='\\') avr_gcc_dir+='\\';
		}
	}
	char key=0;
	while(true){
		show_menu();
		if(key==27||key=='q') break;
		else if(key=='s') rozmiar_ihex();
		else if(key=='c') kompiluj(avr_gcc_dir);
		else if(key=='t') usbasp_test(avr_gcc_dir);
		else if(key=='w') write_program(avr_gcc_dir);
		else if(key=='e') edit_source(edytor);
		else if(key=='d') open_directory();
		else if(key=='1') cmd_exec(cmd1);
		else if(key=='2') cmd_exec(cmd2);
		else if(key=='3') cmd_exec(cmd3);
		cout<<"> ";
		key=getch();
	}
	return 0;
}
