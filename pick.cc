#include <iostream>
#include <iterator>
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <google/protobuf/text_format.h>
#include "training_record.pb.h"

namespace fs = boost::filesystem;

std::string GetCurrentDateTime() {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
	std::string str(buffer);
	return str;
}

void updatePriority(Problem * problem){
	double sum=0.0;
	for (int i = 0 ; i < problem->record_size(); ++i) {
		sum+=problem->record(i).duration();
	}
	problem->set_priority((int)(sum/problem->record_size()));
}

//Check whether all the problems have been studied, if not randomly pick one from the problem not yet studied
//if the function returns true, then do not launch anything.
//if not, randomly pick a question.
bool IsItARevision( TrainingRecord& training_records/*The training record*/){
	using std::stringstream;
	using std::cout;
	using std::endl;
	using std::string;
	using std::vector;
	bool result = true;
	vector<int> indices_of_not_done;
	for (int i = 0 ; i < training_records.problem_size(); ++i) {
		if (training_records.problem(i).priority()==0) {
			result = false;
			indices_of_not_done.push_back(i);
		}
	}
	Problem * problem = training_records.mutable_problem(indices_of_not_done[rand()%indices_of_not_done.size()]);
	string random_pick = problem->name();
	if (!result){
		cout << "There are still problems that have never been tried"<<endl;
		cout << "Checking if processor is available...";
		if (system(NULL)){
			cout << "OK" << endl;
		}else{
			exit (EXIT_FAILURE);
		}
		cout << "Launch gedit..."<< endl;
		boost::timer::cpu_timer chronometer;
		stringstream command;
		command << "gedit "<<random_pick;
		int result = system (command.str().c_str());
		cout << "The value returned "<< result << endl;	
		boost::timer::cpu_times const elapsed_time(chronometer.elapsed());
		string walltime = boost::timer::format(elapsed_time, 10, "%w"); 
		cout << "The time elapsed: "<<boost::timer::format(elapsed_time) << endl;	
		string currentDateTime = GetCurrentDateTime();
		double duration = atof(walltime.c_str());
		Record * record = problem->add_record();
		record->set_date(currentDateTime);
		record->set_duration(duration);
		updatePriority(problem);
	}	
	return result;
}

//Pick the problems according to their priority
void RandomPickWithPriority(TrainingRecord & training_records){
	using std::stringstream;
	using std::cout;
	using std::endl;
	using std::string;
	using std::vector;
	int sum=0;
	for (int i = 0 ; i < training_records.problem_size(); ++i) {
		sum += training_records.problem(i).priority();
	}
	int random = rand()%sum;
	int sum2=0;
	int index_of_chosen;
	for (index_of_chosen=0; index_of_chosen < training_records.problem_size(); ++index_of_chosen) {
		sum2+= training_records.problem(index_of_chosen).priority();
		if (sum2 > random) {
			break;
		}
	}
	Problem * problem = training_records.mutable_problem(index_of_chosen);
	string random_pick = problem->name();
	cout << "Start revision..."<<endl;	
	cout << "Checking if processor is available...";
	if (system(NULL)){
		cout << "OK" << endl;
	}else{
		exit (EXIT_FAILURE);
	}
	cout << "Launch gedit..."<< endl;
	boost::timer::cpu_timer chronometer;
	stringstream command;
	command << "gedit "<<random_pick;
	int result = system (command.str().c_str());
	cout << "The value returned "<< result << endl;	
	boost::timer::cpu_times const elapsed_time(chronometer.elapsed());
	string walltime = boost::timer::format(elapsed_time, 10, "%w"); 
	cout << "The time elapsed: "<<boost::timer::format(elapsed_time) << endl;	
	string currentDateTime = GetCurrentDateTime();
	double duration = atof(walltime.c_str());
	Record * record = problem->add_record();
	record->set_date(currentDateTime);
	record->set_duration(duration);
	updatePriority(problem);

}

void DfsConstruct (const fs::path & file_path, TrainingRecord& training_records){
	using namespace fs;
	using namespace std;
	if (is_regular_file(file_path) ) {
		cout <<"Add new problem: "<<file_path <<" ..."<< endl;
		Problem * problem = training_records.add_problem();
		stringstream name_stream;
		name_stream << file_path;
		problem->set_name(name_stream.str());
		problem->set_priority(0);
			
	}else if(is_directory(file_path) ){
		typedef vector<path> vec;             // store paths,
		vec subpaths;                                // so we can sort them later
		copy(directory_iterator(file_path), directory_iterator(), back_inserter(subpaths));

		for (unsigned int i = 0 ; i < subpaths.size(); ++i) {
			DfsConstruct (subpaths[i], training_records);
		}
	}else{
		cout << file_path << " exists, but is neither a regular file nor a directory"<<endl;
	}
}

struct ProblemCompare {
	bool operator()(const Problem& pb1, const Problem& pb2){
		if(pb1.name().compare(pb2.name())<0){
			return true;
		}else{
			return false;
		}
	}
	bool operator()(const Problem& pb1, const std::string & problem_name){
		if(pb1.name().compare(problem_name)<0){
			return true;
		}else{
			return false;
		}
	}
	bool operator()(const std::string & problem_name, const Problem & pb2){
		if(problem_name.compare(pb2.name())<0) {
			return true;
		}else{
			return false;
		}
	}
};

void SortProblems(TrainingRecord & training_records){
	std::sort (training_records.mutable_problem()->begin(),
		training_records.mutable_problem()->end(), ProblemCompare());
}

bool SearchProblems(TrainingRecord & training_records, const std::string & problem_name) {
	return std::binary_search(training_records.mutable_problem()->begin(),
		training_records.mutable_problem()->end(), problem_name,ProblemCompare());
}

void CheckRecordsAgainstProblemBase(const fs::path & file_path, TrainingRecord& training_records){
	using namespace fs;
	using namespace std;
	if (is_regular_file(file_path) ) {
		stringstream name_stream;
		name_stream << file_path;
		if(!SearchProblems(training_records,name_stream.str())){
			cout << "Add new problem: "<<name_stream.str()<<" ..."<<endl;
			Problem * problem = training_records.add_problem();
			problem->set_name(name_stream.str());
			problem->set_priority(0);
		}

	}else if(is_directory(file_path) ){
		typedef vector<path> vec;             // store paths,
		vec subpaths;                                // so we can sort them later
		copy(directory_iterator(file_path), directory_iterator(), back_inserter(subpaths));

		for (unsigned int i = 0 ; i < subpaths.size(); ++i) {
			CheckRecordsAgainstProblemBase (subpaths[i], training_records);
		}
	}else{
		cout << file_path << " exists, but is neither a regular file nor a directory"<<endl;
	}
}

void RandomPick (const fs::path & file_path) { // based on dfs, the distribution is uniform
	using namespace fs;
	using namespace std;
	if (is_regular_file(file_path) ) {
		cout << file_path << endl;
		cout << "Checking if processor is available...";
		if (system(NULL)){
			cout << "OK" << endl;
		}else{
			exit (EXIT_FAILURE);
		}
		cout << "Launch gedit..."<< endl;
		boost::timer::cpu_timer chronometer;
		stringstream command;
		command << "gedit "<<file_path;
		int result = system (command.str().c_str());
		cout << "The value returned "<< result << endl;	
		boost::timer::cpu_times const elapsed_time(chronometer.elapsed());
		string walltime = boost::timer::format(elapsed_time, 10, "%w"); 
		cout << "The time elapsed: "<<boost::timer::format(elapsed_time) << endl;	
	}else if(is_directory(file_path) ){
		typedef vector<path> vec;             // store paths,
		vec subpaths;                                // so we can sort them later
		copy(directory_iterator(file_path), directory_iterator(), back_inserter(subpaths));

		RandomPick (subpaths[rand()%subpaths.size()]);
	}else{
		cout << file_path << " exists, but is neither a regular file nor a directory"<<endl;
	}
}

int main (int argc, char * argv[]) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	using ::std::cout;
	using ::std::cerr;
	using ::std::endl;
	using fs::path;
	using fs::filesystem_error;
	using ::std::fstream;
	using ::std::ios;
	srand(time(NULL));
	if (argc < 3){//add the record base later as the second argument.The record base will be based on google protocol buffer.
		cout <<"Usage: " <<argv[0]<< " "<<"question_base"<<" record_database" <<endl;
	}else{
		cout <<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<endl;
		//check whether record database exist. if not, software will create a new one.
		path directory_path (argv[1]);
		path record_database_path(argv[2]);
		TrainingRecord training_records;
		try{
			if (!exists(directory_path)){
				cerr << directory_path << " does not exists"<< endl;
				return -1;
			}
			if (exists(record_database_path)) {
				//load the database into memory and check consistency
				if (is_regular_file(record_database_path)) {//check consistency
					fstream record_database_input (argv[2], ios::in|ios::binary);
					training_records.ParseFromIstream(&record_database_input);
					SortProblems (training_records);//sort the problem list in order that search can be faster
					CheckRecordsAgainstProblemBase(directory_path, training_records);//check whether directory has new problems that records do not cover; 
													//check whether records cover the problem that does not exist anymore in the problem base
				}else {
					cerr << "Error: "<<record_database_path<<" is not a regular file."<< endl;
					return -1;
				}	
			}else{
				//create the new database in memory
				//construct the new database
				DfsConstruct (directory_path, training_records);
			}
			//if not all the problems have already been solved at least once, pick problems uniformly randomly from the problems that have not been solved yet.
			if (IsItARevision(training_records)) {
				RandomPickWithPriority(training_records);
			}
			//RandomPick(directory_path);

		}catch (const filesystem_error& ex){
			cerr << ex.what() << endl;
			return -1;
		}
		//store training record into database
		fstream record_database_output (argv[2], ios::out | ios::trunc | ios::binary);
		if (!training_records.SerializeToOstream(&record_database_output)) {
			cerr << "Failed to save training record."<<endl;
			return -1;
		}
		std::string text_message;
		google::protobuf::TextFormat::PrintToString(training_records, &text_message);
		cout << text_message<< endl;
	}
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
