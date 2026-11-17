#include <iostream>
#include <cstdlib>

int main()
{
    int choice = 0;
    while (true) {
        std::cout << "\n=== RUFUS Main Menu ===\n";
        std::cout << "1. Beamforming (rufus-beamformer)\n";
        std::cout << "2. B-mode (rufus-b-mode)\n";
        std::cout << "3. Doppler modes (rufus-doppler)\n";
        std::cout << "4. Elastography (rufus-elasto)\n";
        std::cout << "5. Phantom Generator (rufus-phantom-generator)\n";
        std::cout << "0. Exit\n";
        std::cout << "Choose mode: ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            system("beamformer.exe");
            break;
        case 2:
            system("b-mode.exe");
            break;
        case 3:
            system("doppler.exe");
            break;
        case 4:
            system("elasto.exe");
            break;
        case 5:
            system("phantom-generator.exe");
            break;
        case 0:
            return 0;
        default:
            std::cout << "Invalid choice.\n";
        }
    }
}