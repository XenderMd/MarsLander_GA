// MarsLander.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"

#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <utility>
#include <valarray>
#include <vector>

#if __cplusplus >= 201103L
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <forward_list>
#include <future>
#include <initializer_list>
#include <mutex>
#include <random>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <system_error>
#include <thread>
#include <tuple>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#endif
//#include <bits/stdc++.h>

using namespace std;

#define pi 3.14159265359
#define geneNum 20
#define POPULATION_SIZE 20 

struct Distance
{
	float dx;
	float dy;

	//tollerance limit - how close is close enough to get there. 
	float delta;
};

class Point
{

protected:

	float x;
	float y;

public:

	Point()
	{
		x = 0.0;
		y = 0.0;
	}

	Point(float m, float n)
	{
		x = m;
		y = n;
	}



	void SetX(float m)
	{
		x = m;
	}
	void SetY(float n)
	{
		y = n;
	}

	float GetX() { return x; }
	float GetY() { return y; }
};

class Surface
{
	Point * surface;
	Point lz[2];
	int numPoints;
	int backIndex;

public:

	Surface()
	{
		numPoints = 0;
		surface = new Point[1000];
		backIndex = 0;
		lz[0] = Point();
		lz[1] = Point();
	}

	Surface(int N)
	{
		numPoints = N;
		surface = new Point[N];
		backIndex = 0;
		lz[0] = Point();
		lz[1] = Point();

	}
	~Surface() { delete[] surface; }

	Surface(Surface const& copy)
	{
		numPoints = copy.numPoints;
		surface = new Point[numPoints];
		lz[0] = copy.lz[0];
		lz[1] = copy.lz[1];

		// Don't need to worry about copying integers.
		// But if the object has a copy constructor then
		// it would also need to worry about throws from the copy constructor.
		std::copy(&copy.surface[0], &copy.surface[numPoints], surface);
	}

	Surface & operator=(Surface rhs) // Pass by value (thus generating a copy)
	{
		rhs.swap(*this); // Now swap data with the copy.
						 // The rhs parameter will delete the array when it
						// goes out of scope at the end of the function
		return *this;
	}

	void swap(Surface& s) noexcept
	{
		using std::swap;
		swap(this->surface, s.surface);
		swap(this->numPoints, s.numPoints);
	}

	void SetPoint(Point P)
	{
		if (backIndex < numPoints)
		{
			surface[backIndex] = P;
			backIndex++;
		}
	}

	void SetLZ()
	{
		for (int i = 0; i < (numPoints - 1); i++)
		{
			if (surface[i].GetY() == surface[i + 1].GetY())
			{
				lz[0].SetY(surface[i].GetY());
				lz[0].SetX(surface[i].GetX());
				lz[1].SetY(surface[i + 1].GetY());
				lz[1].SetX(surface[i + 1].GetX());
			}
		}
	}

	Distance GetDistanceToLZ(Point &P)
	{
		Distance LZDistance;

		LZDistance.dx = abs(P.GetX() - (lz[1].GetX() + lz[0].GetX()) / 2);
		LZDistance.dy = abs(P.GetY() - lz[0].GetY());
		LZDistance.delta = ((lz[1].GetX() - lz[0].GetX()) / 2);

		//cerr<<"LZ1: "<< lz[1].GetX() <<" LZ2: "<<lz[2].GetX()<<endl;

		return LZDistance;
	}

	Point GetDirTallestPeak(Point &P)
	{
		Point Peak;

		for (int i = 0; i < numPoints; i++)
		{
			if ((surface[i].GetX() >= P.GetX() && surface[i].GetX() <= lz[0].GetX()) || (surface[i].GetX() <= P.GetX() && surface[i].GetX() >= lz[1].GetX()))
			{
				if (surface[i].GetY() > Peak.GetY())
				{
					Peak = surface[i];
				}
			}
		}

		return Peak;
	}

};

class Lander : public Point
{
private:

	double vx;
	double vy;
	int power;
	int angle;
	double marsG = 3.75;
	int fuel;

private:

	void SetPower(int val)
	{
		power = val;
	}
	void SetAngle(int val)
	{
		angle = val;
	}
	void ApplyFuelConsumption()
	{
		fuel = fuel - power;
	}
	void ApplyPower()
	{
		vx = vx + (-power * sin(angle*pi / 180));
		vy = vy + (-marsG + power * cos(angle*pi / 180));
	}
	void CalcNewPosition()
	{
		x = x + vx;
		y = y + vy;
	}

public:

	Lander(int x, int y) :Point(x, y)
	{
		vx = 0;
		vy = 0;
		power = 0;
		angle = 0;
	}

	void SetFuel(int val) { fuel = val; }
	void SetVx(int m) { vx = (double)m; }
	void SetVy(int n) { vy = (double)n; }
	int GetVx() { return lround(vx); }
	int GetVy() { return lround(vy); }
	int GetFuel() { return fuel; }
	int getPower() { return power; }
	int getAngle() { return angle; }



	void ApplyCommands(int Angle, int Power)
	{
		SetPower(Power);
		SetAngle(Angle);
		ApplyPower();
		ApplyFuelConsumption();
		CalcNewPosition();
	}

};

//encodes direction or the delta modification to be applied to the angle or the power for a given turn.
class Gene
{
	int dangle;
	int dpower;

public:
	
	Gene()
	{
		dangle = 15;
		dpower = 1;
	}

	Gene(int seed)
	{
		//srand(seed);

		float p;
		p = (rand() % 100);
		if (p <= 50) { dangle = -(rand() % 15); }
		else if (p>50) { dangle= (rand() % 15); }
		

		p = (rand() % 100);
		if (p <= 33) { dpower = -1; }
		else if (p > 33 && p <= 66) { dpower = 0; }
		else { dpower = 1; }
	}
		
	void mutate()
	{
		//srand(time(0));

		float p;
		p = (rand() % 100);
		if (p <= 33) { dangle = -15; }
		else if (p > 33 && p <= 66) { dangle = 0; }
		else { dangle = 15; }

		p = (rand() % 100);
		if (p <= 33) { dpower = -1; }
		else if (p > 33 && p <= 66) { dpower = 0; }
		else { dpower = 1; }
	}

	int getAngle() { return dangle; }
	int getPower() { return dpower; }
};

class Chromosome
{
	Gene genes[geneNum];
	int initialangle;
	int initialpower;
	int angles[geneNum] {0};// initial angle - to be used later with the calculation of the consecutive angle values
	int powers[geneNum] {0};// initial power - to be used later with the calculation of the consecutive power values

public:

	Chromosome()
	{

	}

	Chromosome(int angle, int power) // Initializes the genes array, and calculates a succession of angles and powers based on the genes and initial values for the angle and power
	{
		initialangle = angle;
		initialpower = power;

		for (int i = 0; i < geneNum; i++)
		{
			genes[i] = Gene(time(0));

			if (i == 0) 
			{
				angles[i] = initialangle + genes[i].getAngle();

				if (angles[i] > 90) { angles[i] = 90; }
				else if (angles[i] < -90) { angles[i] = -90; }

				powers[i] = initialpower + genes[i].getPower();

				if (powers[i] > 4) { powers[i] = 4; }
				else if (powers[i] < 0) { powers[i] = 0; }
			}
			else
			{
				angles[i] = angles[i - 1] + genes[i].getAngle();
				
				if (angles[i] > 90) { angles[i] = 90; }
				else if (angles[i] < -90) { angles[i] = -90; }

				powers[i] = powers[i - 1] + genes[i].getPower();
				
				if (powers[i] > 4) { powers[i] = 4; }
				else if (powers[i] < 0) { powers[i] = 0; }
			}
		}
	}

	void SetGene(Gene g, int i)
	{
		genes[i] = g;

		if (i == 0)
		{
			angles[i] = initialangle + genes[i].getAngle();

			if (angles[i] > 90) { angles[i] = 90; }
			else if (angles[i] < -90) { angles[i] = -90; }

			powers[i] = initialpower + genes[i].getPower();

			if (powers[i] > 4) { powers[i] = 4; }
			else if (powers[i] < 0) { powers[i] = 0; }
		}
		else
		{
			angles[i] = angles[i - 1] + genes[i].getAngle();

			if (angles[i] > 90) { angles[i] = 90; }
			else if (angles[i] < -90) { angles[i] = -90; }

			powers[i] = powers[i - 1] + genes[i].getPower();

			if (powers[i] > 4) { powers[i] = 4; }
			else if (powers[i] < 0) { powers[i] = 0; }
		}

	}

	Gene GetGene(int i)
	{
		return genes[i];
	}


	int getInitAngle() { return initialangle; }
	int getInitPower() { return initialpower; }
	int getAngle(int i) { return angles[i]; }
	int getPower(int i) { return powers[i]; }
};

class Individual
{
	Chromosome xy;
	int fitness;

public:

	Individual()
	{
		xy = Chromosome(0, 0);
		fitness = 0;
	}

	Individual(int angle, int power)
	{
		xy = Chromosome(angle, power); 
		fitness = 0;
	}

	bool operator < (Individual other)
	{
		return this-> fitness < other.getFitness();
	}

//#TODO Work on a proper implementation of the fitness function...so far there is a small score difference between good and bad solutions

	void calcFitness(Surface Mars, Lander MarsLander)
	{
		//variables used to calculate by how much we are actually appraoching the LZ through the genes of a given individual
		int deltaX = Mars.GetDistanceToLZ(MarsLander).dx;
		int deltaY = Mars.GetDistanceToLZ(MarsLander).dy;

		for (int i = 0; i < geneNum; i++)
		{
			MarsLander.ApplyCommands(xy.getAngle(i), xy.getPower(i));
			/*cout << "X: " << MarsLander.GetX() << " " << "Y: " << MarsLander.GetY() << endl;
			cout << "dX: " << Mars.GetDistanceToLZ(MarsLander).dx << " dY: " << Mars.GetDistanceToLZ(MarsLander).dy << endl;
			cout << "Power: " << xy.getPower(i) << " Angle: " << xy.getAngle(i) << endl;
			cout << "----------------------------------------------------------------------------" << endl;
		   */
		}

		//final calculation of how much we actually approached the LZ after applying the genes of a given individuals 
		deltaX = deltaX - Mars.GetDistanceToLZ(MarsLander).dx;
		deltaY = deltaY - Mars.GetDistanceToLZ(MarsLander).dy;

		
		int distanceToLZX = Mars.GetDistanceToLZ(MarsLander).dx;
		int distanceToLZY = Mars.GetDistanceToLZ(MarsLander).dy;
		int landerHeight = MarsLander.GetY();
		int tallestPeak = Mars.GetDirTallestPeak(MarsLander).GetY();
		int fuel = MarsLander.GetFuel();
		int vx = MarsLander.GetVx();
		int vy = MarsLander.GetVy();
		int tallestpeackratio = (int)(1000.0 * (landerHeight - tallestPeak) / (1 + landerHeight));
		int speedXratio = (int)(10 * (20 - vx) / (1 + distanceToLZX));
		int speedYratio = (int)10 * (40 - vy) / (1 + distanceToLZY);

		fitness = 7000 - distanceToLZX + deltaX; //- dLZY; //+ tallestpeackratio; //+ fuel + speedXratio + speedYratio;
		
		
		//cout << "SpeedX: " << vx << " SpeedY: " << vy << endl;
		
		/*cout << "dX: " << dLZX << " dY: " << dLZY << endl;
		cout << fitness << endl;
		cout << "----------------------------------------------------------------------------" << endl;
	   */
	}

	int getFitness()
	{
		return fitness;
	}

	Chromosome getChromosome()
	{
		return this->xy;
	}
	
	void setGene (Gene g, int i)
	{
		xy.SetGene(g, i);
	}

	Individual mate(Individual other)
	{
		Individual child(xy.getInitAngle(), xy.getInitPower());

		srand(time(0));
		float p;

		for (int i = 0; i < geneNum; i++)
		{
			p = (rand() % 100);
			//cout << p << endl;

			if (p < 35)
			{
				child.setGene(this->getChromosome().GetGene(i), i);
				//cout << "parent 1" << endl;
			}
			else if (p < 70)
			{
				child.setGene(other.getChromosome().GetGene(i), i);
				//cout << "parent2" << endl;
			}
			else
			{
				Gene Mutation;
				Mutation.mutate();
				child.setGene(Mutation, i);
				//cout << "mutation" << endl;
			} 
		}
		
		return child;
	}

	int getAngle(int i) { return xy.getAngle(i); }
	int getPower(int i) { return xy.getPower(i); }
};

//#TODO
class Population
{
	Individual individuals[POPULATION_SIZE];
	int populationfitness=0;


private:

	void sortindiv()
	{
		for (int i = 0; i < POPULATION_SIZE; i++)
			for (int j = i + 1; j < POPULATION_SIZE; j++)
			{
				if (! (individuals[j] < individuals[i]) )
				{
					Individual tmp = individuals[i];
					individuals[i] = individuals[j];
					individuals[j] = tmp;
				}

			}
	}

	void calcpplfitness()
	{
		populationfitness = 0;
		for (int i = 0; i < POPULATION_SIZE; i++)
			populationfitness += individuals[i].getFitness();
	}

public:

	Population()
	{
		for (int i = 0; i < POPULATION_SIZE; i++)
		{
			individuals[i] = Individual();
		}

	}

	Population(int angle, int power)
	{
		//cout << angle<<endl;
		for (int i = 0; i < POPULATION_SIZE; i++)
		{
			individuals[i] = Individual(angle, power);
		}
	}

	int getMaxIndividualFitness()
	{
		return this->individuals[0].getFitness();
	}

	//calculates individuals and then population fitness; sorts individuals according to their population
	void calcfitness(Surface Mars, Lander MarsLander)
	{
		for (int i = 0; i < POPULATION_SIZE; i++)
		{
			individuals[i].calcFitness(Mars, MarsLander);
		}

		// sort individuals in ascending order according to their fitness
		sortindiv();

		// calculate total population fitness
		calcpplfitness();
	}

	// Roullete selection here
	Individual rouletteselection()
	{
		Individual Parent;
		int FP=rand() % populationfitness;
		int PartSum = 0;
		bool found = false;

		for (int i = 0; i<POPULATION_SIZE && !found; i++)
		{
			PartSum += this->individuals[i].getFitness();
			found = PartSum >= FP;
		}

		return Parent;
	}

	Population calcNextGen()
	{
		int elite = (10.0 / 100.0) * POPULATION_SIZE;
		Population nextGen;
		Individual parent1;
		Individual parent2;

		//perform elitism
		for (int i = 0; i < elite; i++)
		{
			nextGen.individuals[i] = this->individuals[i];
		}


		// apply roullette selection for the rest of the population
		for (int i = elite; i < POPULATION_SIZE; i++)
		{
			parent1 = rouletteselection();
			parent2 = rouletteselection();
			nextGen.individuals[i] = parent1.mate(parent2);
		}
		return nextGen;
	}

	void calcNumNextGenSolutions(int Num, Surface Mars, Lander MarsLander)
	{
		for (int i = 0; i < Num; i++)
		{
			this->calcfitness(Mars, MarsLander);
			//cout << "Max Individual Fitness is: " << this->individuals[POPULATION_SIZE-1].getFitness() << endl;
			*this = calcNextGen();
		}

		this->calcfitness(Mars, MarsLander);
	}

};

int main()
{
	
	Surface Mars(7);
	Mars.SetPoint(Point(0, 100));
	Mars.SetPoint(Point(1000, 500));
	Mars.SetPoint(Point(1500, 1500));
	Mars.SetPoint(Point(3000, 1000));
	Mars.SetPoint(Point(4000, 150));
	Mars.SetPoint(Point(5500, 150));
	Mars.SetPoint(Point(6999, 800));
	Mars.SetLZ();


	Lander MarsLander = Lander(2500, 2700);
	MarsLander.SetVx(0);
	MarsLander.SetVy(0);
	MarsLander.SetFuel(544);

	string userInput = "y";

	MarsLander.ApplyCommands(0, 0);

	Population Solutions(MarsLander.getAngle(), MarsLander.getPower());
	Solutions.calcfitness(Mars, MarsLander);
	cout << "(0) Max Individual Fitness is: " << Solutions.getMaxIndividualFitness() << endl;


	// game loop
	while (userInput=="y") {
		
		//Designed to test my game physics simulation//
	/*	MarsLander.ApplyCommands(3, -20);
		cout << "HSpeed: " << MarsLander.GetVx() << " " << "VSpeed: " << MarsLander.GetVy() << endl;
		cout << "X= " << MarsLander.GetX() << "m " << "Y= " << MarsLander.GetY() << "m " << endl;
		cout << "Fuel: " << MarsLander.GetFuel() << endl;
		cout << "Distance to LZ -> dX: " << Mars.GetDistanceToLZ(MarsLander).dx << " dY: " << Mars.GetDistanceToLZ(MarsLander).dy << " Delta: " << Mars.GetDistanceToLZ(MarsLander).delta << endl;
		cout << "Tallest Peak: -> X: " << Mars.GetDirTallestPeak(MarsLander).GetX() << " Y: " << Mars.GetDirTallestPeak(MarsLander).GetY() << endl;
		cout << "------------------------------------------------------------------------- ->>>"<< endl;
	*/

		
		Solutions.calcNumNextGenSolutions(100, Mars, MarsLander);

		cout << "Max Individual Fitness is: " << Solutions.getMaxIndividualFitness() << endl;

		cin >> userInput; cout << endl;

	

		// rotate power. rotate is the desired rotation angle. power is the desired thrust power.
		//cout << "-20 3" << endl;
	}
}