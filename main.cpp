#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <random>

using namespace std;

// Константы для оформления
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string BOLD = "\033[1m";

struct CardDetails {
    string rank;
    string suit;
    string color;
};

// Функция подсчета очков
int calculateScore(const vector<int>& hand) {
    int score = 0, aces = 0;
    for (int card : hand) {
        int rank = card % 13;
        if (rank == 0) { score += 11; aces++; }
        else if (rank >= 10) score += 10;
        else score += (rank + 1);
    }
    while (score > 21 && aces > 0) { score -= 10; aces--; }
    return score;
}

// Получение данных о карте (масть, номинал, цвет)
CardDetails getCardDetails(int card) {
    string suits[] = {"♠", "♥", "♦", "♣"};
    string ranks[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    string color = (card / 13 == 1 || card / 13 == 2) ? RED : RESET;
    return {ranks[card % 13], suits[card / 13], color};
}

// Отрисовка стола
void render(const vector<int>& pHand, const vector<int>& dHand, bool hide, int balance, int bet) {
    system("clear");
    cout << YELLOW << "====================================================" << RESET << endl;
    cout << BOLD << "  CASINO BLACKJACK " << RESET << " | Баланс: " << GREEN << "$" << balance << RESET << " | Ставка: " << GREEN << "$" << bet << RESET << endl;
    cout << YELLOW << "====================================================" << RESET << endl;

    auto drawRow = [&](const vector<int>& hand, bool isHidden) {
        string rows[5] = {"", "", "", "", ""};
        for (int i = 0; i < hand.size(); i++) {
            if (isHidden && i == 1) {
                rows[0] += BLUE + "┌─────────┐ " + RESET;
                rows[1] += BLUE + "│░░░░░░░░░│ " + RESET;
                rows[2] += BLUE + "│░░  ?  ░░│ " + RESET;
                rows[3] += BLUE + "│░░░░░░░░░│ " + RESET;
                rows[4] += BLUE + "└─────────┘ " + RESET;
            } else {
                CardDetails d = getCardDetails(hand[i]);
                string sp = (d.rank == "10") ? "" : " ";
                rows[0] += d.color + "┌─────────┐ " + RESET;
                rows[1] += d.color + "│ " + d.rank + sp + "      │ " + RESET;
                rows[2] += d.color + "│    " + d.suit + "    │ " + RESET;
                rows[3] += d.color + "│       " + sp + d.rank + "│ " + RESET;
                rows[4] += d.color + "└─────────┘ " + RESET;
            }
        }
        for (const string& row : rows) cout << row << endl;
    };

    cout << "\n" << YELLOW << " КАРТЫ ДИЛЕРА:" << RESET << endl;
    drawRow(dHand, hide);
    
    cout << "\n" << BLUE << " ВАШИ КАРТЫ (Очки: " << BOLD << calculateScore(pHand) << RESET << "):" << endl;
    drawRow(pHand, false);
    cout << YELLOW << "----------------------------------------------------" << RESET << endl;
}

void saveRecord(int balance) {
    ofstream file("record.txt");
    if (file.is_open()) {
        file << "Последний баланс: $" << balance << endl;
        file.close();
    }
}

int main() {
    // Инициализация честного рандома
    random_device rd;
    mt19937 g(rd());

    int balance = 1000;
    vector<int> deck(52);
    for (int i = 0; i < 52; i++) deck[i] = i;

    while (balance > 0) {
        shuffle(deck.begin(), deck.end(), g);
        int bet;
        cout << "\nБаланс: $" << balance << ". Введите ставку: ";
        cin >> bet;

        if (bet > balance || bet <= 0) {
            cout << RED << "Ошибка: некорректная ставка!" << RESET << endl;
            continue;
        }

        vector<int> pHand, dHand;
        int dIdx = 0;

        // Анимированная раздача
        pHand.push_back(deck[dIdx++]); render(pHand, dHand, false, balance, bet); usleep(300000);
        dHand.push_back(deck[dIdx++]); render(pHand, dHand, true, balance, bet); usleep(300000);
        pHand.push_back(deck[dIdx++]); render(pHand, dHand, true, balance, bet); usleep(300000);
        dHand.push_back(deck[dIdx++]); render(pHand, dHand, true, balance, bet);

        // Ход игрока
        while (calculateScore(pHand) < 21) {
            cout << " [" << BOLD << "1" << RESET << "] Еще  |  [" << BOLD << "2" << RESET << "] Стоп: ";
            int move; cin >> move;
            if (move == 1) {
                pHand.push_back(deck[dIdx++]);
                render(pHand, dHand, true, balance, bet);
            } else break;
        }

        int pScore = calculateScore(pHand);
        if (pScore <= 21) {
            render(pHand, dHand, false, balance, bet); // Открываем карту дилера
            while (calculateScore(dHand) < 17) {
                usleep(700000);
                dHand.push_back(deck[dIdx++]);
                render(pHand, dHand, false, balance, bet);
            }
        }

        render(pHand, dHand, false, balance, bet);
        int dScore = calculateScore(dHand);

        cout << BOLD;
        if (pScore > 21) { cout << RED << " >>> ПЕРЕБОР! ПРОИГРЫШ <<<" << RESET; balance -= bet; }
        else if (dScore > 21 || pScore > dScore) { cout << GREEN << " >>> ПОБЕДА! +$" << bet << " <<<" << RESET; balance += bet; }
        else if (pScore < dScore) { cout << RED << " >>> ДИЛЕР ВЫИГРАЛ! -$" << bet << " <<<" << RESET; balance -= bet; }
        else { cout << YELLOW << " >>> НИЧЬЯ! <<<" << RESET; }
        cout << endl;

        saveRecord(balance);

        if (balance <= 0) {
            cout << RED << "Вы обанкротились! Игра окончена." << RESET << endl;
            break;
        }
        
        cout << "\nПродолжить (y/n)? ";
        char next; cin >> next;
        if (next == 'n') break;
    }
    return 0;
}
