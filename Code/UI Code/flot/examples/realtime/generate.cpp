#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
using namespace std;

int main()
{
    int random_integer;
    srand((time_t)time(0));
    std::ofstream myfile;
    myfile.open ("output.txt");
    for(int index=0; index<500; index++){
        random_integer = (rand()%100)+1;
        myfile << random_integer << "\n";
    }
    myfile.close();
    system("pause");
    return 0;
}
