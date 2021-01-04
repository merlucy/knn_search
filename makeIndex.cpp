#include <time.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <array>
#include <map>

using namespace std;

const double x_min = -90.0;
const double x_max = 90.0;
const double y_min = -176.3;
const double y_max = 177.5;

class Location{
	public:
		double latitude;
		double longitude;
		Location(double lat, double lon): latitude(lat), longitude(lon){}
		
		bool operator< (const Location &locationObj) const{
			
			if(locationObj.latitude < this->latitude || locationObj.latitude == this->latitude && locationObj.longitude < this->longitude){
				return true;
			}else{
				return false;
			}
		}
		

};

int duplicate_elimination(char * data_path, char * data_path_new)
{
	// read the original dataset from data_path
	// eliminate duplicates by deleting the corresponding lines
	// write the dataset without duplicates into data_path_new

	ifstream file;
	file.open(data_path);

	string latitude, longitude, locationID;
	string flush;
	map<Location, string> dataMap = map<Location, string>();
	int counter = 0;

	while(file >> flush >> flush >> latitude >> longitude >> locationID){
		double latdouble = stod(latitude);
		double londouble = stod(longitude);

		if(latdouble < x_min || latdouble > x_max){
			continue;
		}

		if(londouble < y_min || londouble > y_max){
			continue;
		}

		Location location = Location(latdouble, londouble);

		map<Location, string>::iterator itr = dataMap.find(location);

		if(itr != dataMap.end()){

			if(stoi(dataMap[location]) > stoi(locationID)){
				itr->second = locationID;
			}
			
			continue;
		}
			
		dataMap.insert({ location, locationID});
		cout.precision(14);
		counter += 1;
	}

	file.close();

	//cout << "Deduplicated Data with " << counter << " values created." << endl;
	ofstream newfile;
	newfile.open(data_path_new);
		
	for(map<Location, string>::iterator itr = dataMap.begin(); itr != dataMap.end(); itr++){
		newfile.precision(14);
		newfile << itr->first.latitude << ' ' << itr->first.longitude << ' ' << itr->second << endl;
	}

	newfile.close();

	return 0;
}

class LocationWithID{
	public:
		double latitude;
		double longitude;
		string locationID;

		LocationWithID(double lat, double lon, string locID):latitude(lat), longitude(lon), locationID(locID){}
};

string create_index(char * data_path_new, char * index_path, int n)
{
	// To create a grid index and save it to file on "index_path".
	// The output file should contain exactly n*n lines. If there is no point in the cell, just leave it empty after ":".
	
	ifstream file;
	file.open(data_path_new);

	double horizontal_size = (x_max - x_min) / n;	//actually latitude runs vertically, with -90 degrees to 90 degrees.
	double vertical_size = (y_max - y_min) / n;

	const size_t nVal = n;

	vector<LocationWithID> ***dataArray = new vector<LocationWithID>**[n];

	for(int i = 0; i < n; i++){
		dataArray[i] = new vector<LocationWithID>*[n];
		for(int j = 0; j < n; j++){
			dataArray[i][j] = new vector<LocationWithID>();
		}
	}

	string latitude;
	string longitude;
	string locationID;

	while(file >> latitude >> longitude >> locationID){
		double latdouble = stod(latitude);
		double londouble = stod(longitude);

		int horizontal_box_num = (latdouble - x_min) / horizontal_size;
		int vertical_box_num = (londouble - y_min) / vertical_size;

		int index_x;
		int index_y;

		index_x = horizontal_box_num;
		index_y = n - vertical_box_num - 1;

		if((((latdouble - x_min)/ horizontal_size) - double(index_x) == 0.0 && index_x != 0) || index_x == n){
			index_x -= 1;
		}

		if(index_y == -1){
			index_y += 1;
		}

		dataArray[index_x][index_y]->push_back(LocationWithID(stod(latitude), stod(longitude), locationID));
	}

	file.close();

	ofstream newfile;
	newfile.open(index_path);
	newfile.precision(14);

	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			newfile << "Cell " << j << ',' << i << ": ";
			for(vector<LocationWithID>::iterator itr = dataArray[i][j]->begin(); itr != dataArray[i][j]->end(); itr++){
				newfile << itr->locationID << '_' << itr->latitude << '_'<< itr-> longitude << ' ';
			}
			newfile << endl;			
		}
	}

	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			delete dataArray[i][j];
		}
		delete[] dataArray[i];
	}
	delete[] dataArray;
	newfile.close();
	
	return "";
}

int main(int argc, char ** argv)
{
	if (argc != 5){
		cerr << "Ussage: " << argv[0] << " DATA_PATH INDEX_PATH DATA_PATH_NEW N" << endl;
		/*
			DATA_PATH(char *): the file path of Gowalla_totalCheckins.txt
			INDEX_PATH(char *): the output file path of the grid index
			DATA_PATH_NEW(char *): the file path of the dataset without duplicates
			N(integer): the grid index size
  		*/
		return -1;
	}
	duplicate_elimination(argv[1], argv[3]);
	clock_t s, t;
	s=clock();
	create_index(argv[3], argv[2], atoi(argv[4]));
	t=clock();
	cout<<"index construction time: "<<(double)(t-s)<<endl;
	return 0;
}