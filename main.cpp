#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <cstdlib>  

using namespace std;

map<string,string> global_map;
// file to string does exactly that... yeah
string file_to_string(string filename){
	ifstream infile(filename.c_str());
	string line;
	string all;
	while(getline(infile,line)){
		all += (line + "\n") ;
	}
	return all;
}
// returns the line at the 
// TODO: make a indicator for the collums!!!!!!!!
string line_to_str(string buffer_name, size_t line, size_t col){
	string buf = global_map[buffer_name];//put global buffer in local buffer
	size_t begin = 0;
	for(size_t i = 0; i<line-1;i++){
		begin = buf.find("\n",begin)+1;
	}
	size_t end = buf.find("\n",begin);
	return buf.substr(begin,end-begin);
}
// returns a string with the buffer, line and collom positon
string loc_to_pos(string buffer_name, size_t loc){
	string buf = global_map[buffer_name];//put global buffer in local buffer
	size_t line = 1;
	size_t collom = 0;
	for(size_t i = 0; i < loc; i++){
		if(buf[i]=='\n'){line++;collom=0;}
		else collom++;
	}
	stringstream ss;
	ss << "In buffer '" << buffer_name << "' at line:" << line << " collom:" << collom 
	   << endl << line_to_str(buffer_name,line,collom);
	return ss.str();
}
// tests if st_from_between_st will succeed. if not program will fail.
void test_st_between_st(string buffer_name, string open_str, string close_str, size_t loc){
	string buf = global_map[buffer_name]; //put global buffer in local buffer
	size_t begin = buf.find(open_str,loc);
	if(begin == string::npos){
		cout << "Could not find '" << open_str << "' " << loc_to_pos(buffer_name,loc) << endl;
		exit(-1);
	}
	size_t end = buf.find(close_str,begin+open_str.size());
	if(end == string::npos){
		cout << "Could not find '" << close_str << "' " << loc_to_pos(buffer_name,begin) << endl;
		exit(-1);
	}
}
// collect a string from between two other strings
string st_from_between_st(string buffer_name, string open_str, string close_str, size_t &loc){
	test_st_between_st(buffer_name,open_str,close_str,loc);  // test if the syntax is correct
	string buf = global_map[buffer_name]; //put global buffer in local buffer
	loc = buf.find(open_str,loc)+open_str.size(); //find open_str location 
	size_t nb = buf.find(close_str,loc); // find next location
	string res = buf.substr(loc,nb-loc); // collect string between locations
	loc = nb + close_str.size(); // set location to the end of the close_str location
	return res; // return res... yep.
}

int math_to_int(string input){
	size_t begin = input.find_first_of("0123456789");
	size_t end = input.find_first_not_of("0123456789");
	return stoi(input.substr(begin,end-begin));

}
int math_operator_split(string input){
	size_t loc = input.find_last_of("+-");
	if(loc!=string::npos){
		int l = math_operator_split(input.substr(0,loc));
		int r = math_operator_split(input.substr(loc+1));
		if(input[loc]=='+') return l+r;
		else return l-r;
	} else return math_to_int(input);
}

string math_to_string(string input){
	size_t loc = input.find_first_of("+-");
	if(loc!=string::npos){
		return to_string(math_operator_split(input));
	}
	return to_string(math_to_int(input));
}
// turns open bracket into a closed bracket
string get_other_bracket(char ch){
	if(ch=='{')return "}";
	if(ch=='[')return "]";
	if(ch=='(')return ")";
	return "";
}
// tales a strip {} [] () and returns a string with all the content
string strip_to_string(string buffer_name, size_t &loc){
	string buf = global_map[buffer_name];
	size_t begin = buf.find_first_of("[{(",loc);
	size_t end = buf.find("\n",loc);
	if(begin<end){
		string label = st_from_between_st(buffer_name,string(1,buf[begin]),get_other_bracket(buf[begin]),loc);
		if(buf[begin]=='[') return global_map[label] += strip_to_string(buffer_name,loc);
		else if(buf[begin]=='(') return math_to_string(label) += strip_to_string(buffer_name,loc);
		else return label += strip_to_string(buffer_name,loc);
	}
	loc = buf.find("\n",loc);
	return "";
}
// add to buffer
size_t command_atb(string buffer_name, size_t loc){
	string label = st_from_between_st(buffer_name,"[","]",loc);
	global_map[label] = strip_to_string(buffer_name,loc);
	return loc;
}
// prints buffers and strings
size_t command_print(string buffer_name, size_t loc){
	cout << strip_to_string(buffer_name,loc);
	return loc;
}

size_t flush_buffer(string buffer_name, size_t loc){
	string command = st_from_between_st(buffer_name,"/|","|",loc);
	if(command == "print") loc = command_print(buffer_name,loc);
	else if(command == "atb") loc = command_atb(buffer_name,loc);
	else {cout << "invalid command '" << command << "' " << loc_to_pos(buffer_name,loc-command.size()) << endl;}
	
	if(global_map[buffer_name].find("/|",loc) != string::npos) 
		loc = flush_buffer(buffer_name,loc);
	else return loc;
	return loc;
}

int main(int argc, char const *argv[])
{
	global_map["main"] = file_to_string("testfile.txt");
	flush_buffer("main",0);

	return 0;
}