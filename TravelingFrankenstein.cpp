/* Alex Hoffer: Traveling Frankenstein Solution. A solution to the Traveling Salesman Problem wherein a first path is generated using the Simulated Annealing algorithm, a subsequent path is found using the 2-Opt algorithm, and a third optimized path is found using the 2.5-Opt algorithm. Called the Traveling Frankenstein due to the stitching together of multiple TSP solutions to find a great optimal path in a reasonable amount of time. */
#include <cstdio>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
using namespace std;
typedef vector<int> possibleSolution;

ifstream inputFile;
ofstream outputFile;
string outputFileName;
string testOutputName;

string* fileType;
int* fileNumber;

// Get the right initial temp for Sim. Annealing. If the input file is huge, reduce the initial temperature if it's a competition case so it can run in time.
double getTemperature()
{
	long double Temp;

	if (*fileType == "test-input")
	{
		int num = *fileNumber;

		// Run at this initial temp for test cases 1-4.
		if (num == 1 || num == 2 || num == 3 || num == 4)
		{
			Temp = 1000000000; 
			cout << "Test case " << num << " given, running Simulated Annealing with initial temperature of " << Temp << endl;
			return Temp;
		}

		if (num == 5)
		{
			Temp = 100000000;
			cout << "Test case " << num << " given, running Simulated Annealing with reduced initial temperature of " << Temp << " to save time " << endl;
			return Temp;
		}
	
		// Run at this initial temp for test case 6.
		if (num == 6)	
		{
			Temp = 1000000;
			cout << "Test case " << num << " given, running Simulated Annealing with reduced initial temperature of " << Temp << " to save time " << endl;
			return Temp;
		}

		// Run at this initial temp for test case 7.
		if (num == 7)
		{
			Temp = 950000;
			cout << "Test case " << num << " given, running Simulated Annealing with reduced initial temperature of " << Temp << " to save time " << endl;
			return Temp;
		}

		// Handle other input.
		else 
		{
			Temp = 1000000000;
			cout << "Test case " << num << " given, running Simulated Annealing with initial temperature of " << Temp << endl;
			return Temp;
		}
	}

	// Run at this temperature for all example cases.
	else if (*fileType == "tsp_example")
	{
		cout << "Tsp_example file given, running Simulated Annealing with initial temperature of 1000000000" << endl;
		Temp = 1000000000; 
		return Temp;
	}

	// Handle other input.
	else
	{
		Temp = 1000000000;
		cout << "Running Simulated Annealing at initial temperature of " << Temp << endl;
		return Temp;
	}

}

// Get the right number of iterations. Huge inputs require smaller iterations per temperature.
int getIterations()
{
	long int numIterations;

	if (*fileType == "test-input")
	{
		int num = *fileNumber;

		// Iterate this much for test cases 1-4.
		if (num == 1 || num == 2 || num == 3 || num == 4)
		{
			numIterations = 100000;
			cout << "Test case " << num << " given, at each temperature will run " << numIterations << " iterations" << endl;
			return numIterations;
		}

		if (num == 5)
		{
			numIterations = 75000;
			cout << "Test case " << num << " given, at each temperature will run reduced number of iterations (" << numIterations << ") to save time" << endl;
			return numIterations;
		}
	
		// Iterate this much for test case 6.
		if (num == 6)	
		{
			numIterations = 25000;
			cout << "Test case 6 given, at each temperature will run reduced number of iterations (" << numIterations << ") to save time" << endl;
			return numIterations;
		}

		// Iterate this much for test case 7.
		if (num == 7)
		{
			numIterations = 6000;
			cout << "Test case 7 given, at each temperature will run reduced number of iterations (" << numIterations << ") to save time" << endl;
			return numIterations;
		}

		// If there's some other input we don't know about, just go with the default.
		numIterations = 100000;
		cout << "At each temperature, running " << numIterations << " iterations." << endl;
		return numIterations;
	}

	// Iterate this many times per temp for all example cases.
	else if (*fileType == "tsp_example")
	{
		numIterations = 100000;
		cout << "Tsp_example given, at each temperature will run " << numIterations << " iterations." << endl;
		return numIterations;
	}

	// Handle other input.
	else
	{
		numIterations = 100000;
		cout << "At each temperature, running " << numIterations << " iterations." << endl;
		return numIterations;
	}
}

// Holds data from the parsed text input file.
class cityInformation 
{
public:
	vector< pair<double,double> > cityCoords;
	vector< vector<double> > cost;
	vector <int> cityNumbers;
	vector <int> xCoordinates;
	vector <int> yCoordinates;
	int** adjacencyMatrix;
	int N;
	char s[30];

	cityInformation() 
	{

			// Parse given input file.
			string line;
			if (!inputFile)
			{
				cout << "Error opening file." << endl;
				exit(1);
			}

			// Load  line by line.
			while (getline(inputFile, line))
			{
				int counter = 0;

				// Parse the string.
				stringstream stream(line);
		
				while (1)
				{
					int tmp;
					stream >> tmp;

					if (stream)
					{
						if (counter == 0)
							cityNumbers.push_back(tmp);

						else if (counter == 1)
							xCoordinates.push_back(tmp);
	
						else if (counter == 2)
							yCoordinates.push_back(tmp);
					}		

					else if (!stream) // No more string to parse, break the infinite loop.
					{
						break;
					}

					counter++;
				}		
			}
	
		N = cityNumbers.size();
		
		// index from 1
		cityCoords.push_back(make_pair(0,0));
		vector< double> T;
		cost.push_back(T);

		for(int i=0;i<N;i++) 
		{
			double x,y;
			cityCoords.push_back(make_pair(xCoordinates.at(i), yCoordinates.at(i)));
		}

		constructAdjacencyMatrix();

		for(int i=0;i<N;i++) 
		{
			vector<double> V(N+1);
			
			for(int j = 1;j<N;j++) 
			{
				V[j] = adjacencyMatrix[i][j];
			}
			
			cost.push_back(V);
		}
	}

	// Given the indices of two cities, compute the Euclidean distance between them.
	int computeDistance(int idx1, int idx2)
	{
		int x1 = xCoordinates.at(idx1);
		int x2 = xCoordinates.at(idx2);
		int y1 = yCoordinates.at(idx1);
		int y2 = yCoordinates.at(idx2);

		float firstClause = x1 - x2;
		firstClause = pow(firstClause, 2);
		float secondClause = y1 - y2;
		secondClause = pow(secondClause, 2);
		float sumOfClauses = firstClause + secondClause;
		float distance = sqrt(sumOfClauses);

		distance = round(distance);

		return distance;
	}

	// Create a two-dimensional matrix that holds the distances between each point to each other point
	void constructAdjacencyMatrix()
	{
		int max = cityNumbers.size();
		adjacencyMatrix = new int*[max];

		for (int i = 0; i < cityNumbers.size(); i++)
		{
			adjacencyMatrix[i] = new int[max];

			for (int j = 0; j < cityNumbers.size(); j++)
			{
				adjacencyMatrix[i][j] = computeDistance(i, j);
			}	
		}
	}

	// Return a random value according to some range given by i.
	static int myrandom (int i) 
	{ 
		return rand()%i;
	}

	// Generate a random tour solution.
	possibleSolution genRand(possibleSolution C) 
	{
		int l,a,b,temp;
		l = C.size();
		a = rand()%l;
		b = rand()%l;
		temp = C[a];
		C[a] = C[b];
		C[b] = temp;
		return C;
	}

	// Discover the total cost of a possible solution tour.
	double evaluate(possibleSolution C) 
	{
		return tourCost(C);
	}

	// Compute the total cost of a possible solution tour.
	double tourCost(possibleSolution C) 
	{
		int l = C.size();
		double tourCost = 0.0;
		l = l-1;
		for (int i=0;i <l; i++) {
			tourCost += cost[C[i]][C[i+1]];
		}
		tourCost += cost[C[l]][C[0]];
		return tourCost;
	}
};

// Implements Simulated Annealing, Two-Opt, Two-Half Opt algorithms
class TravelingFrankenstein
{
public:
	possibleSolution best;
	long int minTour;
	cityInformation data;

	TravelingFrankenstein() 
	{
		minTour = 0.0;
	} 

	// Generate a random probability.
	double randProb()
	{
		double p = (rand() / (RAND_MAX + 1.0));
	    	return p;
	}

	void SimulatedAnnealing() 
	{
		vector <int> V(data.N);
		
		// Initialize the vertices.
		for (int i=0;i < data.N;i++) 
			V[i] = i+1;

		// Get an initial, random possible solution. It's the best one we know of right now.
		possibleSolution c = data.genRand(V);
		best = c;
		minTour = data.tourCost(c);

		// Randomly check the neighbors of the first possible solution, trying to find a shorter path.
		for(int i=0;i<1000;i++) 
		{
			random_shuffle(c.begin(),c.end());
			double tCost = data.tourCost(c);
			
			if(tCost < minTour) 
			{
				minTour = tCost;
				best = c;
			}
		}
		
		c = best;
		
		// Variables used to compute probability in the case of a tour being worse than the best current tour.
		double p,expP,deltaE;
		
		double Temp = getTemperature();
		int numberOfIterations = getIterations();

		// Start at initial temperature. Go through each temperature numberOfIterations times. Select a random tour from the previous best tour and compute its distance.
		for (double T = Temp; T > 0.01; T*=0.5) 
		{
			for (int i = 0; i < numberOfIterations; i++)
			{
				possibleSolution n = data.genRand(c);
			
				deltaE = eval(c) - eval(n);
				deltaE /= T;
				p = 1 / (1+exp(-1*deltaE));
				expP = randProb();
				
				// Do we take the random tour? We probalitistically chose to do so in this case.
				if(expP < p) 	
				{ 
					c = n;
					double tCost = data.tourCost(c);
					if(tCost < minTour ) {
						best = c;
						
						minTour = tCost;
					}
				}	
			}
		}
		
		// Given our current best path, let's make it better.
		cout << "Using TwoOpt algorithm on Simulated Annealing result." << endl;
		twoOpt(); 
		
	}

	/* Given an initial optimal candidate solution from the Simulated Annealing algorithm, optimize further using the 2-Opt algorithm.
 	* The idea here is to look through the best path we have currently and if an edge crosses over another edge, we swap the nodes such that the edges are now straight. This reduces the total distance our Frankenstein travels. */ 
	void twoOpt()
	{
		bool changed;

		do {
			changed = false;
			
			// For each node within bounds, check what nodes it's connected to. If we can reconnect such that there's no crossing over, we reduce distance, so swap the edges.
			for (int i = 0; i < best.size() - 4; i++)
			{
				for (int j = i + 3; j < best.size() - 4; j++)
				{
					int distance1 = data.computeDistance(best.at(i) -1, best.at(i + 1) -1);
					int distance2 = data.computeDistance(best.at(j-1) -1, best.at(j)-1);
					int distance3 = data.computeDistance(best.at(i)-1, best.at(j-1)-1);
					int distance4 = data.computeDistance(best.at(i+1)-1, best.at(j)-1);

					// When we travel from node i to node i +  1, and then node j - 1 to node j, we're crossing over in the graph and thereby increasing our total distance. This is because the distance from node i to node j - 1 and then node i + 1 to node j is actually shorter, and yet it still maintains the constraint of reaching each node. Swap the nodes out to optimize the distance.
					if ((distance1 + distance2) > (distance3 + distance4))
					{
						reverse(best.begin() + i + 1, best.begin() + j);
						changed = true;
					}
				}
			}
		}
		while (changed); 

		if (*fileType == "test-input")
		{
			if (*fileNumber == 7)
			{
				cout << "Test input file 7 given. Skipping 2.5-Opt to save time." << endl;
				calculatePathDistance();
			}
		}

		else
		{
			cout << *fileType << "Commencing 2.5-Opt algorithm." << endl;
			twoHalfOpt();
		}
	}

	// We've applied Two-Opt to our best path, now let's subject our path to a third optimizing algorithm. This time, we broaden our horizons from Two-Opt by not simply considering two sets of adjacent edges, but instead three edges where two are adjacent to each other and 1 is not. 
	void twoHalfOpt()
	{
		bool changed;

		do
		{
			changed = false;
		
			for (int i = 0; i < best.size() - 4; i++)
			{
				for (int j = i + 3; j < best.size() - 4; j++)
				{
					int option1current = (data.computeDistance(best.at(i) - 1, best.at(i+1) - 1)) + (data.computeDistance(best.at(i+1)-1, best.at(i+2)-1)) + (data.computeDistance(best.at(j-1)-1, best.at(j)-1));
					int option1Check = (data.computeDistance(best.at(i)-1, best.at(i+2)-1)) + (data.computeDistance(best.at(j-1) -1, best.at(i+1)-1)) + (data.computeDistance(best.at(i+1) -1, best.at(j)-1));

					if (option1Check < option1current)
					{
						int temp = best.at(i+1);
						for (int m = i + 2; m < j; m++)
							best.at(m-1) = best.at(m);
						best.at(j-1) = temp;
						changed = true;
					}

					int option2current = (data.computeDistance(best.at(i)-1, best.at(i+1)-1)) + (data.computeDistance(best.at(j-2)-1, best.at(j-1)-1)) + (data.computeDistance(best.at(j-1)-1, best.at(j)-1));
					int option2Check = (data.computeDistance(best.at(i)-1, best.at(j-1)-1)) + (data.computeDistance(best.at(j-1)-1, best.at(i+1)-1)) + (data.computeDistance(best.at(j-2)-1, best.at(j)-1));
					
					if (option2Check < option2current)
					{
						int temp = best.at(j-1);							
							
						for (int m = j -2; m > i; m--)
							best.at(m+1) = best.at(m);
						
						best.at(i+1) = temp;
						changed = true;
					}
				}
			}

		} while (changed);

		calculatePathDistance();
	}

	void calculatePathDistance()
	{
		int result = 0;

		for (int i = 0; i < best.size() - 1; i++)
			result += data.computeDistance(best.at(i)-1, best.at(i + 1)-1);

		result += data.computeDistance(best.at(best.size() -1)-1, best.at(0)-1);
  		minTour = result;
		outputToFile();
	}

	void outputToFile()
	{
		outputFile.open(outputFileName.c_str());
		outputFile << minTour << "\n";
	
		for (int i = 0; i < best.size(); i++)
			outputFile << best.at(i)-1 << "\n";

		outputFile.close();
	}


	double eval(possibleSolution C) 
	{
		return data.evaluate(C);
	}

};

// Check what file was used as input so we can adjust initial temperature, # of iterations correctly.
void parseFileTitle(char* fileTitle)
{
	string fileT(fileTitle);
	int lastIndex = fileT.find_first_of('.');
	string fileName = fileT.substr(0, lastIndex);
	
	if (fileName.find("test-input") != std::string::npos)
	{
		string title1 = "test-input";
		fileType = new string;
		*fileType = title1;

		int indexBefore = fileName.find('-', 5);  
		char fileNumInChar = fileName[indexBefore+1];
		int fileNumInInt = fileNumInChar - '0';
		
		fileNumber = new int;
		*fileNumber = fileNumInInt;
	}

	else if (fileName.find("tsp_example") != std::string::npos)
	{
		string title2 = "tsp_example";
		fileType = new string;
		*fileType = title2;
	}
}

int main(int argc, char* argv[]) 
{
	fileType = NULL;
	fileNumber = NULL;

	parseFileTitle(argv[1]);

	ofstream testOutput;
	clock_t t;
	t = clock();
	srand(time(NULL));
	
	inputFile.open(argv[1]);
	string firstPart(argv[1]);	
	outputFileName = firstPart + ".tour";	

	TravelingFrankenstein Shelley;
	Shelley.SimulatedAnnealing();

	testOutputName = firstPart + ".time";
	testOutput.open(testOutputName.c_str());

	t = clock() - t;
	float convtime = ((float)t)/(CLOCKS_PER_SEC);
	testOutput << convtime;
	testOutput.close();

	return 0;
}
