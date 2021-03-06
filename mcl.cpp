#include <vector>
#include <cmath>
#include <math.h>
#include <iostream>
#include <cstdlib>
using namespace std;

class particle {
public:
	int pose[3];
	int weight;

public:
	void setPose(int newPose[3]);
	void setWeight(int weight);
	void getPose();
};
class control {
public:
	int Tvel;
	int Rvel;
	int duration;

	control(int T, int R, int D);

};
class feature {
public:
	int range;
	int bearing; //degrees?
	int signiture; //signiture 
	int correspondence;
	int x;
	int y;
public:
	void create(int r, int b, int s, int c, int x_in, int y_in);
};
class map {
public:
	static vector<feature> Map;

	map();

	static void populateMap(int numOfFeatures);
};
vector<particle> generateParticles(vector<particle> particles, int setSize);
vector<particle> mcl(vector<particle> inParticles, control movement, int sampleSize);
particle motion_model(particle p_new, particle previous, control move);
int  MeasurmentModel(feature feature, particle p, map Map);
int prob(int delta);
//-----------------------------------------------------------------------------------------------------------------------------------
	void particle::setPose(int newPose[3]){
		pose[0] = newPose[0];//update x
		pose[1] = newPose[1];//update y
		pose[2] = newPose[2];//update theta
	}
	void particle::setWeight(int weight){
		weight = this->weight;


	}
	void particle::getPose() {
		cout << "x: " << pose[0];
		cout << " y: " << pose[1];
		cout << " t: " << pose[2];
		
	}
//-----------------------------------------------------------------------------------------------------------------------------------
	control::control(int T, int R, int D) {
		Tvel = T;
		Rvel = R;
		duration = D;
	}
//-----------------------------------------------------------------------------------------------------------------------------------
	void feature:: create(int r, int b, int s, int c, int x_in, int y_in) {
	range = r;
	bearing = b;
	signiture = s;
	correspondence = c;
	x = x_in;
	y = y_in;

}
//-----------------------------------------------------------------------------------------------------------------------------------
	vector<feature> map::Map;
	map::map() {
	populateMap(10);
}
	void map::populateMap(int numOfFeatures) {

	for (int i = 0; i < numOfFeatures; i++) {
		feature f;
		f.create(i, i, i, i, i, i);
		Map.push_back(f);
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------

int main(){
	vector<particle> particles;
	particles = generateParticles(particles,10);
	control movement(1, 1, 1);
	int sampleSize = 10;
	vector<particle> newParticles;
	for (int i = 0; i<particles.size(); i++){
		particles.at(i).getPose();
		cout<<"\n";
	}
	newParticles = mcl(particles, movement, sampleSize);
	cout<<"\n";
	for (int i = 0; i<newParticles.size(); i++){
		newParticles.at(i).getPose();
		cout<<" weight: "<< newParticles.at(i).weight <<"\n";
	}
	return 0;
}
//-----------------------------------------------------------------------------------------------------------------------------------

vector<particle> generateParticles(vector<particle> particles, int setSize) {
	for (int i = 0; i < setSize; i++) {
		int x = rand() % 15;//randoms 0-15
		int y = rand() % 15;
		int t = rand() % 15;
		particle p;
		int pose[3] = { x,y,t };
		p.setPose(pose);
		particles.push_back(p);
	}

	return particles;

}

//-----------------------------------------------------------------------------------------------------------------------------------

vector<particle> mcl(vector<particle> inParticles ,control movement, int sampleSize){
	vector<particle> predSample;
	vector<particle> resample;
	map new_map;
	
	predSample.clear();
	resample.clear();
	//predicitive sampling
	particle p;
	int pose[3] = {0,0,0};
	int weight = 0;
	p.setPose(pose);
	p.setWeight(weight);

	for (int i = 0; i < sampleSize; i++) {
		p = motion_model(p, inParticles.at(i), movement);

		p.weight = MeasurmentModel(new_map.Map.at(i), p, new_map);//p has pose, map.at(i) has feature and correspondence, Map is the map
		predSample.push_back(p);
	}
	for(int i =0; i < sampleSize-3; i++){
		//draw i with prob proportional with w[i]
		particle selected = predSample.at(0);
		int location = 0;
		for (int j = 1; j < predSample.size(); j++) {
			if (predSample.at(j).weight >= selected.weight) {
				selected = predSample.at(j);
				location = j;
			}
		}
		resample.push_back(selected);
		predSample.at(location).weight = -1;//sets weight so it wont be selected again
	}
	return resample;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//Velocity Motion Model
//positive rotation is left
//positive translation is forward 
particle motion_model(particle p_new, particle previous, control move){
 

int xCenter;
int yCenter;

xCenter = (previous.pose[0] - (move.Tvel/move.Rvel)*sin(previous.pose[2]));
yCenter = (previous.pose[1] + (move.Tvel/move.Rvel)*cos(previous.pose[2]));

p_new.pose[0] = xCenter + (move.Tvel/move.Rvel)*sin(previous.pose[2]+ (move.Rvel*move.duration));
p_new.pose[1] = yCenter - (move.Tvel/move.Rvel)*cos(previous.pose[2]+ (move.Rvel*move.duration));
p_new.pose[2] = move.Rvel*move.duration;


return p_new;
}

//-----------------------------------------------------------------------------------------------------------------------------------

int  MeasurmentModel(feature feature,particle p, map Map){//occupancy grid map???

	int j = feature.correspondence;
		int tRange; // r-hat
		
		tRange = sqrt(
			((Map.Map.at(j).x-p.pose[0])*(Map.Map.at(j).x-p.pose[0]))
			+((Map.Map.at(j).y-p.pose[1])*(Map.Map.at(j).y-p.pose[1])));

		int tBearing; //Phi-hat
		
		tBearing = atan2((Map.Map.at(j).y-p.pose[1]),(Map.Map.at(j).x-p.pose[0]));
		

		int q; //numerical probablity p(f[i] at time t | c[i] at time t, m, x at time t)
		int deltaR, deltaB, deltaS;
		deltaR = feature.range - tRange;
		deltaB = feature.bearing - tBearing;
		deltaS = feature.signiture - Map.Map.at(j).correspondence;
		int error = rand() % 30;

		q = (prob(deltaR) + prob(deltaB) + prob(deltaS))/3;
		q -= error;
		return q; 
}
int prob(int delta) {
	int prob;
	prob = 100 - (delta / 100); //100% - delta(percentage)
	return prob;
}
