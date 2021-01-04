#include <time.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <cmath>
#include <queue>
#include <list>
#include <algorithm>
#include <cstdlib>

using namespace std;

const double x_min = -90.0;
const double x_max = 90.0;
const double y_min = -176.3;
const double y_max = 177.5;

struct LocationQuery{
	double latitude;
	double longitude;
	LocationQuery(double lat, double lon):latitude(lat), longitude(lon){}
};

vector<LocationQuery> createQuery(int count){
	vector<LocationQuery> v = vector<LocationQuery>();

	for(int i = 0; i < count; i++){
		double latitude = ((double)rand() / RAND_MAX)*(x_max - x_min) + x_min;
		double longitude = ((double)rand() / RAND_MAX)*(y_max - y_min) + y_min;

		v.push_back(LocationQuery(latitude, longitude));
	}
		
	return v;
}

class LocationWithID{
	public:
		double latitude;
		double longitude;
		string locationID;

		LocationWithID(double lat, double lon, string locID):latitude(lat), longitude(lon), locationID(locID){}
};

struct PointDistance{
	double distance;
	double latitude;
	double longitude;
	string locationID;

	PointDistance(double lat, double lon, double dist, string locID):latitude(lat), longitude(lon), distance(dist), locationID(locID){}
};

struct BoxIndex{
	int x_index;
	int y_index;
	BoxIndex(int x, int y):x_index(x), y_index(y){}

	bool operator<(BoxIndex const & b1) const {
		return b1.x_index < this->x_index || b1.x_index == this->x_index && b1.y_index < this->y_index;
	}
};

set<BoxIndex> calculateBoxIndexes(int x_index, int y_index, int layer, int top_limit){

	set<BoxIndex> boxIndex;

	int x_min_limit = x_index - layer;
	int x_max_limit = x_index + layer;
	int y_min_limit = y_index - layer;
	int y_max_limit = y_index + layer;

	for(int i = x_min_limit; i <= x_max_limit; i++){

		if(i < 0 || i > top_limit){
			continue;
		}

		if(y_min_limit < 0 && y_max_limit > top_limit){
			break;
		}else if(y_min_limit < 0){
			boxIndex.insert(BoxIndex(i, y_max_limit));
			continue;
		}else if(y_max_limit > top_limit){
			boxIndex.insert(BoxIndex(i, y_min_limit));
			continue;
		}

		boxIndex.insert(BoxIndex(i, y_max_limit));
		boxIndex.insert(BoxIndex(i, y_min_limit));
	}

	for(int i = y_min_limit; i <= y_max_limit; i++){

		if(i < 0 || i > top_limit){
			continue;
		}

		if(x_min_limit < 0 && x_max_limit > top_limit){
			break;
		}else if(x_min_limit < 0){
			boxIndex.insert(BoxIndex(x_max_limit, i));
			continue;
		}else if(x_max_limit > top_limit){
			boxIndex.insert(BoxIndex(x_min_limit, i));
			continue;
		}

		boxIndex.insert(BoxIndex(x_min_limit, i));
		boxIndex.insert(BoxIndex(x_max_limit, i));
	}

	return boxIndex;
}

struct ComparePoint{
	bool operator()(PointDistance const & p1, PointDistance const & p2){
		return p1.distance < p2.distance;
	}
};

double euclidean_distance(double x_1, double y_1, double x_2, double y_2){

	return sqrt(pow((x_2 - x_1), 2) + pow((y_2 - y_1), 2));
}

bool box_check(int box_x, int box_y, int point_box_x, int point_box_y, double point_x, double point_y, double top, int n){
	double distance_to_box = 0.0;
	double horizontal_size = (x_max - x_min) / n;	//actually latitude runs vertically, with -90 degrees to 90 degrees.
	double vertical_size = (y_max - y_min) / n;

	if(point_box_x == box_x){
		if(point_box_y < box_y){
			distance_to_box = point_y - (y_max - (box_y)*vertical_size);
		}else{
			distance_to_box = (y_max - (box_y + 1)*vertical_size) - point_y;
		}
	}else if(point_box_y == box_y){
		if(point_box_x < box_x){
			distance_to_box = (x_min + (box_x)*horizontal_size) - point_x;
		}else{
			distance_to_box = point_x - (x_min + (box_x + 1)*horizontal_size);
		}
	}else if(point_box_x > box_x && point_box_y > box_y){
		//compared box on left top
		distance_to_box = euclidean_distance(point_x, point_y, (x_min + (box_x + 1)*horizontal_size), (y_max - (box_y + 1)*vertical_size));

	}else if(point_box_x < box_x && point_box_y > box_y){
		//compared box on right top
		distance_to_box = euclidean_distance(point_x, point_y, (x_min + (box_x)*horizontal_size), (y_max - (box_y + 1)*vertical_size));
	
	}else if(point_box_x > box_x && point_box_y < box_y){
		//compared box on left bottom
		distance_to_box = euclidean_distance(point_x, point_y, (x_min + (box_x + 1)*horizontal_size), (y_max - (box_y)*vertical_size));

	}else if(point_box_x < box_x && point_box_y < box_y){
		//compared box on right bottom
		distance_to_box = euclidean_distance(point_x, point_y, (x_min + (box_x)*horizontal_size), (y_max - (box_y)*vertical_size));
	}

	if(distance_to_box > top){
		return false;
	}else{
		return true;
	}
}

PointDistance get_top_from_vector(vector<PointDistance> v){

	return v.back();
}

string knn_grid(double x, double y, char * index_path, int k, int n)
{
	// to get the 5-NN result with the help of the grid index
	// Please store the 5-NN results by a String of location ids, like "11, 789, 125, 2, 771"

	ifstream file;
	file.open(index_path);
	cout.precision(14);

	vector<LocationWithID> ***dataArray = new vector<LocationWithID>**[n];

	for(int i = 0; i < n; i++){
		dataArray[i] = new vector<LocationWithID>*[n];
		for(int j = 0; j < n; j++){
			dataArray[i][j] = new vector<LocationWithID>();
		}
	}

	string cell;
	string flush;
	string index;
	string line;
	int counter = 0;

	while(getline(file, line)){

    	istringstream iss(line);
		iss >> flush >> index;

		int commaPosition = index.find(',');
		int colonPosition = index.find(':');

		int y_index = stoi(index.substr(0, commaPosition));
		int x_index = stoi(index.substr(commaPosition + 1, colonPosition - commaPosition - 1));

		string point;

		while(iss >> point){

			int first_us = point.find('_');
			int second_us = point.find('_', first_us + 1);

			string locationID = point.substr(0, first_us);
			string latitude = point.substr(first_us + 1, second_us - first_us - 1);
			string longitude = point.substr(second_us + 1, point.length() - second_us - 1);

			counter += 1;
			dataArray[x_index][y_index]->push_back(LocationWithID(stod(latitude), stod(longitude), locationID));
		}
	}

    //vector implementation
	/*
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			for(vector<LocationWithID>::iterator itr = dataArray[i][j]->begin(); itr != dataArray[i][j]->end(); itr++){
				//cout << itr->locationID << ' ' << itr->latitude << ' '<< itr-> longitude << endl;
			}
		}
	}
	*/

	double query_x = x;
	double query_y = y;

	double horizontal_size = (x_max - x_min) / n;	//actually latitude runs vertically, with -90 degrees to 90 degrees.
	double vertical_size = (y_max - y_min) / n;
	int horizontal_box_num = (query_x - x_min) / horizontal_size;
	int vertical_box_num = (query_y - y_min) / vertical_size;

	int index_x;
	int index_y;

	index_x = horizontal_box_num;
	index_y = n - vertical_box_num - 1;

	if(((query_x - x_min)/horizontal_size - double(index_x) == 0.0 && index_x != 0) || index_x == n){
		index_x -= 1;
	}

	if(index_y == -1){
		index_y += 1;
	}
	
	priority_queue<PointDistance, vector<PointDistance>, ComparePoint> pq;

    //vector declaration for vector implementation of kNN container
	//vector<PointDistance> result_vector = vector<PointDistance>();

	int layer = 0;
	int box_counter = 0;
	int cumul_box_counter = 0;

	while(true){

		set<BoxIndex> boxIndexes = calculateBoxIndexes(index_x, index_y, layer, n - 1);

		for(set<BoxIndex>::iterator box_itr = boxIndexes.begin(); box_itr != boxIndexes.end(); box_itr++){
			
			if(!pq.empty() && !box_check(box_itr->x_index, box_itr->y_index, index_x, index_y, x, y, pq.top().distance, n)){
				box_counter += 1;
			}

			if(dataArray[box_itr->x_index][box_itr->y_index]->size() != 0){

				for(vector<LocationWithID>::iterator itr = dataArray[box_itr->x_index][box_itr->y_index]->begin(); 
					itr != dataArray[box_itr->x_index][box_itr->y_index]->end(); itr++){

					double euc_distance = euclidean_distance(x, y, itr->latitude, itr->longitude);	

					if(pq.size() >= k && pq.top().distance < euc_distance){

						continue;
					}

					pq.push(PointDistance(itr->latitude, itr->longitude, euc_distance, itr->locationID));

				}

                int queue_size = pq.size();
                for(int i = 0; i < queue_size - k; i++){
                    pq.pop();
                }
			}
		}

		if(box_counter == boxIndexes.size() && pq.size() == k){
			break;
		}else if(cumul_box_counter >= (n*n)){
			break;
		}else{
			cumul_box_counter += box_counter;
			box_counter = 0;
		}

		layer += 1;
	}

	while(!pq.empty()){
		PointDistance p = pq.top();
		pq.pop();

		if(pq.empty()){
			cout << p.locationID;
		}else{
			cout << p.locationID << ", ";
		}
	}

	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			delete dataArray[i][j];
		}
		delete[] dataArray[i];
	}
	delete[] dataArray;

	file.close();
	
	return "";
}

string knn_linear_scan(double x, double y, char * data_path_new, int k)
{
	// to get the 5-NN result by linear scan
	// Please store the 5-NN results by a String of location ids, like "11, 789, 125, 2, 771"
	clock_t a = clock();
	
	ifstream file;
	file.open(data_path_new);
	cout.precision(14);
	
	string latitude;
	string longitude;
	string locationID;

	vector<LocationWithID> dataArray = vector<LocationWithID>();

	priority_queue<PointDistance, vector<PointDistance>, ComparePoint> pq;

    //vector implementation of kNN container
	//vector<PointDistance> result_vector = vector<PointDistance>();

	while(file >> latitude >> longitude >> locationID){
		double latdouble = stod(latitude);
		double londouble = stod(longitude);

		dataArray.push_back(LocationWithID(latdouble, londouble, locationID));
	}

	double distance = 0;

	for(vector<LocationWithID>::iterator itr = dataArray.begin(); itr != dataArray.end(); itr++){

		distance = euclidean_distance(x, y, itr->latitude, itr->longitude);

		if(pq.size() >= k){
			if(distance > pq.top().distance){
				continue;
			}
		}

		pq.push(PointDistance(itr->latitude, itr->longitude, distance, itr->locationID));

        int queue_size = pq.size();
        for(int i = 0; i < queue_size - k; i++){
            pq.pop();
        }
	}

	while(!pq.empty()){
		PointDistance p = pq.top();
		pq.pop();

		if(pq.empty()){
			cout << p.locationID;
		}else{
			cout << p.locationID << ", ";
		}
	}
	
	return "";
}

int main(int argc, char** argv)
{
	if (argc != 7){
		cerr << "Usage: " << argv[0] << " X Y DATA_PATH_NEW INDEX_PATH K N" << endl;
		/*
			X(double): the latitude of the query point q
			Y(double): the longitude of the query point q
			DATA_PATH_NEW(char *): the file path of dataset you generated without duplicates
			INDEX_PATH(char *): the file path of the grid index
			K(integer): the k value for k-NN search
			N(integer): the grid index size
  		*/
		return -1;
	}
	clock_t s, t;
	s=clock();
  	cout << "Linear scan results: "<<knn_linear_scan(atof(argv[1]), atof(argv[2]), argv[3], atoi(argv[5]))<<endl;
	t=clock();
	cout<<"Linear scan time: "<<(double)(t-s)<<endl;
	
	s=clock();
  	cout << "Grid index search results: "<<knn_grid(atof(argv[1]), atof(argv[2]), argv[4], atoi(argv[5]), atoi(argv[6]))<<endl;
	t=clock();
	cout<<"Grid index search time: "<<(double)(t-s)<<endl;
	
	return 0;
}
