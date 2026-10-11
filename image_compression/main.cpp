#include <iostream>
#include <string>
#include <limits>
#include <exception>

#include "compression/encoder.h"
#include "compression/decoder.h"

using namespace std;

int main()
{
    int choice;

    while (true)
    {
        cout << "\n========== IMAGE COMPRESSION TOOL ==========\n";
        cout << "1. Compress Image\n";
        cout << "2. Decompress Image\n";
        cout << "0. Exit\n";
        cout << "============================================\n";
        cout << "Enter choice: ";

        if (!(cin >> choice))
        {
            if (cin.eof())
                break;

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number.\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0)
        {
            cout << "Exiting...\n";
            break;
        }

        if (choice != 1 && choice != 2)
        {
            cout << "Invalid choice. Try again.\n";
            continue;
        }

        string inputFile, outputFile;

        cout << "Enter input filename: ";
        if (!getline(cin, inputFile) || inputFile.empty())
        {
            cout << "Invalid input filename.\n";
            continue;
        }

        cout << "Enter output filename: ";
        if (!getline(cin, outputFile) || outputFile.empty())
        {
            cout << "Invalid output filename.\n";
            continue;
        }

        try
        {
            if (choice == 1)
                compressImage(inputFile.c_str(), outputFile.c_str());
            else
                decompressImage(inputFile.c_str(), outputFile.c_str());

            cout << "Operation completed successfully.\n";
            cout << "Input:  " << inputFile << '\n';
            cout << "Output: " << outputFile << '\n';
        }
        catch (const exception &e)
        {
            cerr << "Error: " << e.what() << '\n';
        }
    }

    return 0;
}