#include <iostream>
#include <cstdlib>

int main()
{
    int choice = 0;
    while (true) {
        std::cout << "\n=== RUFUS Main Menu ===\n";
        std::cout << "1. Synthetic Aperture (rufus-sa)\n";
        std::cout << "2. Doppler modes (rufus-doppler)\n";
        std::cout << "3. Elastography (rufus-elasto)\n";
        std::cout << "4. Phantom Generator (rufus-phantom-generator)\n";
        std::cout << "0. Exit\n";
        std::cout << "Choose mode: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            system("rufus-sa.exe");
            break;
        case 2:
            system("rufus-doppler.exe");
            break;
        case 3:
            system("rufus-elasto.exe");
            break;
        case 4:
            system("rufus-phantom-generator.exe");
            break;
        case 0:
            return 0;
        default:
            std::cout << "Invalid choice.\n";
        }
    }
}