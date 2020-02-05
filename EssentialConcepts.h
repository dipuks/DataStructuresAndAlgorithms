#pragma once
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

#define SIZE 10

class EssentialConcepts
{
	int arr[SIZE];

	struct MyStruct {
		int age;
		string name;
		int id;
	};

	int* ptr;

public:

	//Integer Array Initialization using random values.
	//And the values are printed into the console window.
	void initArray()
	{
		//Array being initialized with random values.
		for (int i = 0; i < SIZE; i++)
		{
			arr[i] = rand();
		}

		//Array values are being printed.
		for (int x : arr)
		{
			cout << "Array = " << x << endl;
		}
	}

	//Struct initialization and printing it's values.
	void initStruct()
	{
		//Struct being initialized
		MyStruct ms;
		ms.age = 23;
		ms.id = 12345;
		ms.name = "Robin";
		
		cout << "Age is " << ms.age << endl;
		cout << "ID is " << ms.id << endl;
		cout << "Name is " << ms.name << endl;

		MyStruct ms1 = {35, "John", 7890};
		cout << "Age is " << ms1.age << endl;
		cout << "ID is " << ms1.id << endl;
		cout << "Name is " << ms1.name << endl;
	}

	//Pointer intialization.
	void initPointer()
	{
		int a = 550;
		ptr = &a;

		cout << "Pointer value is " << *ptr << endl;

		//Dynamic memory allocation for integer pointer.
		int* arr = new int[SIZE];

		//Array being initialized with random values.
		for (int i = 0; i < SIZE; i++)
		{
			arr[i] = rand();
		}

		//Array values are being printed.
		for(int i = 0; i < SIZE; i++)
		{
			cout << "Array = " << arr[i ]<< endl;
		}

		delete[] arr; // Heap memory deleted.
	}

};

