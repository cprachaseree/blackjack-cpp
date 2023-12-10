#include "strategy.h"

void Strategy::read_config(string config_file_path)
{
    string line;
    LOG << "Start reading config file: " << config_file_path << endl;

    ifstream config_file(config_file_path);
    while (getline(config_file, line))
    {
        if (line.substr(0, 2) == "//" || line.length() < 1)
        {
            // skips commented out configs and new lines
            continue;
        }

        // parse each line of the config
        size_t delim_pos = line.find("=");
        string key = remove_whitespace(line.substr(0, delim_pos));
        string value = remove_whitespace(line.substr(delim_pos + 1, line.length()));
        LOG << key << " = " << value << endl;

        // save each value from config file to Config struct variables
        if (key == "strategy_path" || key == "counting_strategy_path")
        {
            // remove starting and ending ""
            if (value.size() > 1)
            {
                if (value[value.size() - 1] == '\"')
                {
                    value = value.substr(0, value.size() - 1);
                }
                if (value[0] == '\"')
                {
                    value = value.substr(1, value.size()-1);
                }
            }
            if (key == "strategy_path")
            {
                this->config.strategy_path = value;
            }
            else if (key == "counting_strategy_path")
            {
                this->config.counting_strategy_path = value;
            }
        }
        else if (key == "num_simulations" || key == "bankroll" || key == "num_decks" || key == "num_hands")
        {
            // check if can convert to integer
            try
            {
                if (key == "num_simulations")
                {
                    this->config.num_simulations = stoi(value);
                } 
                else if (key == "bankroll")
                {
                    this->config.bankroll = stoi(value);
                }
                else if (key == "num_decks")
                {
                    this->config.num_decks = stoi(value);
                }
                else if (key == "num_hands")
                {
                    this->config.num_hands = stoi(value);
                }

            } 
            catch (exception &err)
            {
                LOG << "Conversion failure: " << err.what() << endl;
                // LOG << "Invalid integer for " << value << endl;
                throw "Invalid integer " + value + " for " + key + " in config file.";
            }
        }
        else if (key == "blackjack_bonus")
        {
            // check if can convert to float
            try
            {
                this->config.blackjack_bonus = stod(value);
            }
            catch (exception &err)
            {
                LOG << "Conversion failure: " << err.what() << endl;
                throw "Invalid double " + value + " for " + key + " in config file.";
            }
        }
        else
        {
            throw "Unknown config variable " + key + " in config file.";
        }
    }
    if (this->config.strategy_path == "" || this->config.counting_strategy_path == "" || this->config.num_simulations == 0 || this->config.bankroll == 0 || this->config.blackjack_bonus == 0 || this->config.num_decks == 0 || this->config.num_hands == 0)
    {
        LOG << "Some required config value(s) not initialized." << endl;
        LOG << "strategy_path" << "=" << this->config.strategy_path << endl;
        LOG << "counting_strategy_path" << "=" << this->config.counting_strategy_path << endl;
        LOG << "num_simulations" << "=" << this->config.num_simulations << endl;
        LOG << "bankroll" << "=" << this->config.bankroll << endl;
        LOG << "blackjack_bonus" << "=" << this->config.blackjack_bonus << endl;
        LOG << "num_decks" << "=" << this->config.num_decks << endl;
        LOG << "num_hands" << "=" << this->config.num_hands << endl;
        throw "Missing config variables, please check.";
    }
    config_file.close();
    LOG << "End reading config file: " << config_file_path << endl;
}


void Strategy::read_strategy()
{
    unordered_map<string, unordered_map<string, string>> strategy;
    unordered_set<string> valid_dealer_hands = {"2", "3", "4", "5", "6", "7", "8", "9","10", "A"};
    unordered_set<string> valid_player_hands =
    {
        "21", "20", "19", "18", "17", "16", "15", "14", "13", "12", "11", "10", "9", "8", "7", "6", "5",
        "A;9", "A;8", "A;7", "A;6", "A;5", "A;4", "A;3", "A;2", "A;A", 
        "10;10", "9;9", "8;8", "7;7", "6;6", "5;5", "4;4", "3;3", "2;2"
    };
    string strategy_path = this->config.strategy_path;
    LOG << "Start reading strategy from " << strategy_path << endl;
    ifstream strategy_file(strategy_path);
    if (!strategy_file.is_open())
    {
        strategy_file.close();
        throw  "Invalid strategy path: " + strategy_path;
    }
    string line;

    // first line is dealer cards
    getline(strategy_file, line); 
    vector<string> given_dealer_hands = split_string(line, ",");

    given_dealer_hands = vector<string>(given_dealer_hands.begin() + 1, given_dealer_hands.end());
    if (given_dealer_hands.size() != valid_dealer_hands.size())
    {
        throw "Invalid header for dealer. ,2,3,4,5,6,7,8,9,10,A required.";
    }
    
    for (string dealer_hand : given_dealer_hands)
    {
        LOG << "Dealer columns: " << dealer_hand << endl;
        if (!valid_dealer_hands.count(dealer_hand))
        {
            throw "Invalid column name for dealer face card.";
        }
    }
    vector<string> player_hand_strategies;
    string player_hand;
    int line_count = 1;
    while (getline(strategy_file, line))
    {
        player_hand_strategies = split_string(line, ",");
        string player_hand = player_hand_strategies[0];
        if (!valid_player_hands.count(player_hand))
        {
            throw "Invalid player hand " + player_hand;
        }
        player_hand_strategies = vector<string>(player_hand_strategies.begin() + 1, player_hand_strategies.end());
        if (player_hand_strategies.size() != valid_dealer_hands.size())
        {
            throw "Invalid row in line " + to_string(line_count) + ". Got " + to_string(player_hand_strategies.size()) + " elements while expecting " + to_string(valid_dealer_hands.size());
        }
        
        for (size_t i = 0; i < given_dealer_hands.size(); i++)
        {
            strategy[given_dealer_hands[i]][player_hand] = player_hand_strategies[i];
        }
        line_count++;
        LOG << "End reading line " << line_count << endl;
    }
    LOG << "End reading strategy" << endl;
    strategy_file.close();
    this->strategy = strategy;
}

void Strategy::read_counting_strategy()
{
    unordered_set<string> valid_cards = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "A"};
    string counting_strategy_path = this->config.counting_strategy_path;
    ifstream counting_strategy_file(counting_strategy_path);
    if (!counting_strategy_file.is_open())
    {
        counting_strategy_file.close();
        throw  "Invalid strategy path: " + counting_strategy_path;
    }
    string cards_line;
    string counts_line;
    vector<string> cards;
    vector<string> counts;

    getline(counting_strategy_file, cards_line);
    getline(counting_strategy_file, counts_line);

    cards = split_string(cards_line, ",");
    counts = split_string(counts_line, ",");

    if (cards.size() != valid_cards.size())
    {
        throw "Invalid counting strategy header. 2,3,4,5,6,7,8,9,10,A required.";
    }
    if (counts.size() != valid_cards.size())
    {
        throw "Invalid counting strategy counts. Ten counts required.";
    }
    for (int i = 0; i < cards.size(); i++)
    {
        if (!valid_cards.count(cards[i]))
        {
            throw "Invalid counting strategy header. 2,3,4,5,6,7,8,9,10,A required.";
        }
        this->counting_strategy[cards[i]] = stoi(counts[i]);
    }
    this->current_count = 0;
}

void Strategy::init_deck()
{
    int i;
    LOG << "Starting deck initialization" << endl;
    // initialize single deck;
    this->deck = {"A"};
    for (i = 2; i < 10; i++)
    {
        this->deck.push_back(to_string(i));
    }
    for (i = 0; i < 4; i++)
    {
        this->deck.push_back("10");
    }
    print_vector(this->deck);
    // copy deck num of decks, * 4 for the 4 suits
    size_t single_deck_size = this->deck.size();
    this->deck.resize(this->config.num_decks * single_deck_size * 4);
    vector<string>::iterator start_copy = this->deck.begin();

    LOG << this->config.num_decks << " number of decks " << endl;
    for (i = 0; i < this->config.num_decks * 4; i++)
    {
        copy_n(this->deck.begin(), single_deck_size, start_copy + single_deck_size * i);
    }
    print_vector(this->deck);

    random_device rd;
    auto rng = default_random_engine{rd()};

    shuffle(this->deck.begin(), this->deck.end(), rng);
    print_vector(this->deck);
    LOG << "Final Deck size " << this->deck.size() << endl; 
    LOG << "Ending deck initialization" << endl;

}

string Strategy::combine_player_hands(string first_player_hand, string second_player_hand)
{
    string combined_player_hand;
    if (first_player_hand == "A")
    {
        combined_player_hand = "A;" + second_player_hand;
    }
    else if (second_player_hand == "A")
    {
        combined_player_hand = "A;" + first_player_hand;
    }
    else if (first_player_hand == second_player_hand)
    {
        combined_player_hand = first_player_hand + ";" + second_player_hand;
    }
    else
    {
        combined_player_hand = to_string(stoi(first_player_hand) + stoi(second_player_hand));
    }
    return combined_player_hand;
}

void Strategy::update_hand_value(Hand &hand, string new_card)
{
    if (new_card == "A")
    {
        hand.number_of_a++;
        hand.current_value += 11;
    }
    else
    {
        hand.current_value += stoi(new_card);
    }
    for (int i = 0; i < hand.number_of_a; i++)
    {
        if (hand.current_value > 21)
        {
            hand.current_value -= 10;
        }
    }
}

void Strategy::update_count_value(string seen_card)
{
    this->current_count += this->counting_strategy[seen_card];
}

int Strategy::calc_bet()
{
    // TO DO implement betting calculation based on count this->current_count
    return 1;
}

void Strategy::run_simulation()
{
    LOG << "Start running simulation" << endl;
    if (this->strategy.size() == 0)
    {
        throw "No strategy loaded. Exiting.";
    }
    LOG << "Running simulation for " << this->config.num_simulations << " loops." << endl;

    for (int i = 1; i < this->config.num_simulations + 1; i++) 
    {
        LOG << "simulation " << i << endl;
        this->init_deck();
        this->current_count = 0;
        int bankroll = this->config.bankroll;
        LOG << "Bank roll: " << bankroll << endl;
        for (int j = 0, k = 0; j < this->deck.size() *2 / 3; j += k)
        {
            LOG << "Deck position: " << j << endl;
            Hand dealer_hand;
            vector<Hand> player_hands;

            int bet = this->calc_bet();
            
            LOG << "Current bet: " << bet << endl;
            
            dealer_hand.cards.push_back(this->deck[j + (k++)]);
            dealer_hand.cards.push_back(this->deck[j + (k++)]);

            this->update_hand_value(dealer_hand, dealer_hand.cards[0]);
            this->update_hand_value(dealer_hand, dealer_hand.cards[1]);


            LOG << "Dealer hand: " << dealer_hand.cards[0] << " " << dealer_hand.cards[1] << endl;
            LOG << "Original dealer value: " << dealer_hand.current_value << endl;

            for (int h = 0; h < this->config.num_hands; h++)
            {
                Hand player_hand;
                player_hand.cards.push_back(this->deck[j + (k++)]);
                player_hand.cards.push_back(this->deck[j + (k++)]);
                this->update_hand_value(player_hand, player_hand.cards[0]);
                this->update_hand_value(player_hand, player_hand.cards[1]);
                player_hands.push_back(player_hand);
                LOG << "Player hand: " << player_hand.cards[0] << " " << player_hand.cards[1] << endl;
                LOG << "Original player value: " << player_hand.current_value << endl;
            }

            string &shown_dealer_hand = dealer_hand.cards[0];
            string combined_player_hand;
            
            for (int h = 0; h < player_hands.size(); h++)
            {
                Hand &player_hand = player_hands[h];
                LOG << "Player hand: " << player_hand.cards[0] << " " << player_hand.cards[1] << endl;
                combined_player_hand = this->combine_player_hands(player_hand.cards[0], player_hand.cards[1]);
                LOG << "combined player hand: " << combined_player_hand << endl;
    
                if (combined_player_hand == "A;10")
                {
                    LOG << "BLACKJACK! " << combined_player_hand << endl;
                    bankroll += this->config.blackjack_bonus * bet;
                    player_hands.erase(player_hands.begin() + h);
                    this->update_count_value("A");
                    this->update_count_value("10");
                    continue;
                }

                // keep getting cards until stand
                string player_decision = strategy[shown_dealer_hand][combined_player_hand];
                LOG << "Shown dealer hand: " << shown_dealer_hand << endl;
                LOG << "Player decision: " << player_decision << endl;

                while (player_decision != "S")
                {
                    if (player_decision == "SP")
                    {
                        // split
                        LOG << "split hand" << endl;
                        LOG << "current " << combined_player_hand << endl;
                        if (combined_player_hand == "A;A")
                        {
                            player_hand.current_value -= 1;
                            player_hand.number_of_a--;
                        }
                        else
                        {
                            player_hand.current_value -= stoi(player_hand.cards[1]);
                            
                        }

                        string new_card = this->deck[j + (k++)];
                        player_hand.cards[1] = new_card;
                        this->update_hand_value(player_hand, new_card);
                        
                        Hand new_hand;
                        new_hand.cards.push_back(player_hand.cards[0]);
                        new_hand.cards.push_back(this->deck[j + (k++)]);
                        this->update_hand_value(new_hand, new_hand.cards[0]);
                        this->update_hand_value(new_hand, new_hand.cards[1]);
                        player_hands.push_back(new_hand);
                    }
                    else if (player_decision == "H")
                    {
                        // hit
                        LOG << "hit hand" << endl;
                        string new_card = this->deck[j + (k++)];
                        player_hand.cards.push_back(new_card);
                        this->update_hand_value(player_hand, new_card);
                        LOG << "new card " << new_card << endl;
                        LOG << "new player value " << player_hand.current_value << endl;
                    }
                    else if (player_decision == "U")
                    {
                        // surrender
                        LOG << "surrender hand" << endl;
                        bet /= bet;
                        player_hands.erase(player_hands.begin() + h);
                        for (string card : player_hand.cards)
                        {
                            this->update_count_value(card);
                        }
                        break;
                    }
                    else if (player_decision == "D")
                    {
                        // double down
                        LOG << "double down" << endl;
                        bet *= 2;
                        string next_card = this->deck[j + (k++)];
                        LOG << "added card " << next_card << endl; 
                        player_hand.cards.push_back(next_card);
                        break;
                    }
                    // calculate new player_hand
                    if (player_hand.current_value >= 21)
                    {
                        break;
                    }
                    combined_player_hand = to_string(player_hand.current_value);
                    player_decision = strategy[shown_dealer_hand][combined_player_hand];
                }
            }
            LOG << "Dealer value with 2 cards: " << dealer_hand.current_value << endl;
            while (dealer_hand.current_value < 17)
            {
                string new_card = this->deck[j + (k++)];
                LOG << "Next dealer card " << new_card << endl;
                this->update_hand_value(dealer_hand, new_card);
                LOG << "Next dealer value " << dealer_hand.current_value << endl;;
            }
            for (int h = 0; h < player_hands.size(); h++)
            {
                Hand &player_hand = player_hands[h];
                int player_value = player_hand.current_value;
                int dealer_value = dealer_hand.current_value;

                LOG << "Before dealer deal value: " << dealer_value << endl;
                LOG << "Player final value: " << player_value << endl;

                // compare values
                if (player_value > 21)
                {
                    LOG << "Player BUST! Player value at " << player_value << endl;
                    bankroll -= bet;
                }
                else if (dealer_value > 21)
                {
                    LOG << "Dealer BUST! Dealer value at " << dealer_value << endl;
                    bankroll += bet;
                }
                else if (player_value > dealer_value)
                {
                    LOG << "Win! " << player_value << " > " << dealer_value << endl;
                    bankroll += bet;
                }
                else if (player_value < dealer_value)
                {
                    LOG << "LOSE! " << player_value << " < " << dealer_value << endl;
                    bankroll -= bet;
                }
                else
                {
                    LOG << "Tie! " << player_value << " = " << dealer_value << endl;
                }
            }
            // update this->current_count
            // seen cards in players
            LOG << "Previous count: " << this->current_count << endl;
            LOG << "Seen cards:\n"; 
            for (string card : dealer_hand.cards)
            {
                this->update_count_value(card);
                cout << card << " ";
            }
            for (Hand player_hand : player_hands)
            {
                for (string card : player_hand.cards)
                {
                    this->update_count_value(card);
                    cout << card << " ";
                }
            }
            cout << endl;
            LOG << "New count: " << this->current_count << endl;
        }
        LOG << "Ending bankroll for simulation " << i << ": " << bankroll << endl;
    }
    LOG << "End running simulation" << endl;
}

int Strategy::hands_to_value(vector<string> hands)
{
    int value = 0;
    int number_of_a = 0;
    for (string hand : hands)
    {
        if (hand == "A")
        {
            number_of_a++;
        }
        else
        {
            value += stoi(hand);
        }
    }
    for (int i = 0; i < number_of_a; i++)
    {
        value += 11;
        if (value > 21)
        {
            value -= 10;
        }
    }
    return value;
}