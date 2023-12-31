#include "utils.h"
#include <chrono>
#include <random>

struct Config
{
    string strategy_path = "";
    string counting_strategy_path = "";
    int num_simulations = 0;
    int bankroll = 0;
    int num_decks = 0;
    int num_hands = 0;
    double blackjack_bonus = 0;
};

struct Hand
{
    vector<string> cards;
    int current_value = 0;
    int number_of_a = 0;
};

class Strategy
{
    private:
        Config config;
        vector<string> deck;
        unordered_map<string, unordered_map<string, string>> strategy;
        unordered_map<string, int> counting_strategy;
        int running_count;

    public:
        void read_config(string config_file_path);
        void read_strategy();
        void read_counting_strategy();
        void run_simulation();
        void update_running_count(string seen_card);
        int calc_bet(int deck_op);
        string combine_player_hands(string first_player_hand, string second_player_hand);
        void update_hand_value(Hand &hand, string new_card);
        void init_deck(int seed);
        int hands_to_value(vector<string> hands);
};

