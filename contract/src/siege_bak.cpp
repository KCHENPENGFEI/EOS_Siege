#include "siege.hpp"
#include "function.cpp"


void Siege::allstart()
{
    /* after this action, then all the other actions can be act */
    //require_auth(name);
    //cities_table _cities(_self, _self.value);
    for (uint64_t i = 1; i <= CITY_NUM; i++)
    {
        auto city_it = _cities.find(i);
        if (city_it == _cities.end())
        {
            city_it = _cities.emplace(_self, [&](auto &new_city) {
                new_city.city_id = i;
            });
        }
    }
    // print(name{_self});
    //global_table _global(_self, _self.value);
    auto global_it = _global.find(1);
    if (global_it == _global.end())
    {
        global_it = _global.emplace(_self, [&](auto &new_global) {
          new_global.produce_rate = 0.0;
        });
    }
}

void Siege::transfer(name player_name, name to, asset quantity, string memo)
{
    vector<string> info;
    split(memo, ',', info);
    require_auth(player_name);
    eosio_assert(player_name != to, "cannot transfer to self");
    eosio_assert(to == _self, "only can transfer the contract");
     
    auto sym = quantity.symbol;
    eosio_assert(sym.is_valid(), "invalid symbol name");
    eosio_assert(sym == symbol("ZJUBCA", 4), "invalid symbol name");
    
    eosio_assert(quantity.is_valid(), "invalid quantity");
    //if(info[0].c_str() == "SiegeStart")
    if (!strcmp(info[0].c_str(), "SiegeStrt"))
    {
        //startgame
        //startgame(player_name);
        //Authrozied
        //require_auth(player_name);
        //eosio_assert(from != to, "cannot transfer to self");
        //eosio_assert(to == _self, "only can transfer the contract");
        
        //auto sym = quantity.symbol;
        //eosio_assert(sym.is_valid(), "invalid symbol name");
        //eosio_assert(sym == symbol("ZJUBCA", 4), "invalid symbol name");
        
        //eosio_assert(quantity.is_valid(), "invalid quantity");
        //eosio_assert(quantity.amount > 0, "must transfer positive quantity");

        //players_table _players(_self, _self.value);
        eosio_assert(quantity.amount == ENTER_FEE, "must transfer 2.0000 ZJUBCA to enter the game");
        auto existing = _players.find(player_name.value);
        if (existing == _players.end())
        {
            existing = _players.emplace(player_name, [&](auto &new_player) {
                new_player.player = player_name;
            });
        }

        auto &player_it = _players.get(player_name.value, "Cannot find the player!");
        _players.modify(player_it, player_name, [&](auto &content) {
            game_info game_data;

            //initialize the game data for players
            content.game_data = game_data;
        });

        //auto quantity = asset(ENTER_FEE, TOKEN_SYMBOL);
        //asset enter_fee = asset(ENTER_FEE, symbol("ZJUBCA", 4));
    }
    //else if(info[0].c_str() == "SiegeOccupation")
    else if (!strcmp(info[0].c_str(), "SiegeOccupation"))
    {
        eosio_assert(quantity.amount == OCCUPATION_FEE, "must transfer 1.0000 ZJUBCA to occupy a city");
        //uint64_t city_idx = strtoull(info[1].c_str(), NULL, 0);
        uint64_t city_idx = stoull(info[1].c_str());
        auto &player_it = _players.get(player_name.value, "Cannot find the player!");
        eosio_assert(player_it.is_defender == 0, "You have already occupied another city!");
        
        _players.modify(player_it, player_name, [&](auto &content) {
            content.is_defender = 1;
            content.own_city_id = city_idx;
        });
        
        auto &city_it = _cities.get(city_idx, "Cannot find the city!");
        eosio_assert(city_it.if_be_occupied == 0, "The city has been occupied before.");
        
        _cities.modify(city_it, player_name, [&](auto &content) {
            content.if_be_occupied = 1;
            content.belong_player = player_name;
        });
        
        auto &global_it = _global.get(1, "Error when check global table!");
        eosio_assert(global_it.cities_remain > 0, "No city remained!");
        
        _global.modify(global_it, player_name, [&](auto &content) {
            content.cities_remain -= 1;
            content.produce_rate *= (CITY_NUM - content.cities_remain - 1) / (CITY_NUM - content.cities_remain);
        });
    }
    //else if(info[0].c_str() == "SiegeBuySolidiers")
    else if (!strcmp(info[0].c_str(), "SiegeBuySolidiers"))
    {
        solidier solidier_type = stoull(info[2].c_str());
        double solidier_point = solidiers_point.at(solidier_type);
        double solidier_price = solidier_point * 5.0;    //价格等于点数的5倍，可调整
        eosio_assert(quantity.amount == solidier_price, "price is not match");
        //players_table _players(_self, _self.value);
        auto &player_it = _players.get(player_name.value, "Cannot find the player!");
        eosio_assert(player_it.game_data.status == RUNNING, "Game is over!");
        eosio_assert(player_it.game_data.all_solidiers_point <= 4.0,
                    "You solidier point is max!");
        eosio_assert(player_it.game_data.solidier_quantity < 5,
                    "You solidier quantity is max!");

        _players.modify(player_it, player_name, [&](auto &content) {
            game_info game_data = content.game_data;

            //update the game info
            game_data.all_solidiers_point += solidier_point;
            game_data.current_solidiers_point += solidier_point;
            game_data.solidier_quantity += 1;
            game_data.solidiers_cellar.push_back(solidier_type);

            content.game_data = game_data;
        });
    }
    

    // action(
    //     permission_level{player_name, "active"_n},
    //     "eosio.token"_n, "transfer"_n,
    //     std::make_tuple(player_name, _self, quantity,
    //                     std::string("Enter game!")))
    //     .send();
}

/*void Siege::occupation(name player_name, uint64_t city_idx)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    eosio_assert(player_it.is_defender == 0, "You have already occupied another city!");

    _players.modify(player_it, player_name, [&](auto &content) {
        content.is_defender = 1;
        content.own_city_id = city_idx;
    });

    //cities_table _cities(_self, _self.value);
    auto &city_it = _cities.get(city_idx, "Cannot find the city!");
    eosio_assert(city_it.if_be_occupied == 0, "The city has been occupied before.");

    _cities.modify(city_it, player_name, [&](auto &content) {
        content.if_be_occupied = 1;
        content.belong_player = player_name;
    });

    //global_table _global(_self, _self.value);
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.cities_remain > 0, "No city remained!");

    _global.modify(global_it, player_name, [&](auto &content) {
        content.cities_remain -= 1;
        content.produce_rate *= (CITY_NUM - content.cities_remain - 1) / (CITY_NUM - content.cities_remain);
    });

    //give the occupation fee to the contract
    auto quantity = asset(OCCUPATION_FEE, TOKEN_SYMBOL);

    action(
        permission_level{player_name, "active"_n},
        "eosio.token"_n, "transfer"_n,
        std::make_tuple(player_name, _self, quantity,
                        std::string("You have occupied a city!")))
        .send();
}*/

void Siege::leavecity(name player_name)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");

    uint64_t city_id = player_it.own_city_id;

    //withdraw the produced bonus
    //cities_table _cities(_self, _self.value);
    auto &city_it = _cities.get(city_id, "Cannot find the city!");
    double produced_bonus = city_it.produced_bonus;
    auto quantity = asset((produced_bonus + 0.01) * 10000, symbol("ZJUBCA", 4));     //此处bonus仍然是固定的

    action(
        permission_level{_self, "active"_n},
        "zjubca.token"_n, "transfer"_n,
        std::make_tuple(_self, player_name, quantity,
                        std::string("You have got the bonus and left the city!")))
        .send();

    //clear data
    _players.modify(player_it, player_name, [&](auto &content) {
        content.is_defender = FALSE;
        content.own_city_id = 0;
    });

    _cities.modify(city_it, player_name, [&](auto &content) {
        content.if_be_occupied = FALSE;
        content.belong_player = name(0);
        content.produced_bonus = 0.1;
    });

    //global_table _global(_self, _self.value);
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.cities_remain < 5, "All the city is not occupied!");

    _global.modify(global_it, player_name, [&](auto &content) {
        content.cities_remain += 1;
        if (content.cities_remain == 5)
        {
            content.produce_rate = 0;
        }
        else
        {
            content.produce_rate *= (CITY_NUM - content.cities_remain + 1) / (CITY_NUM - content.cities_remain);
        }
    });
}

/* where the winer's money go? */
/*void Siege::buysolidiers(name player_name, solidier solidier_type)
{
    //Authrozied
    require_auth(player_name);

    double solidier_point = solidiers_point.at(solidier_type);
    players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    eosio_assert(player_it.game_data.status == RUNNING, "Game is over!");
    eosio_assert(player_it.game_data.all_solidiers_point <= 4.0,
                 "You solidier point is max!");
    eosio_assert(player_it.game_data.solidier_quantity < 5,
                 "You solidier quantity is max!");

    _players.modify(player_it, player_name, [&](auto &content) {
        game_info game_data = content.game_data;

        //update the game info
        game_data.all_solidiers_point += solidier_point;
        game_data.current_solidiers_point += solidier_point;
        game_data.solidier_quantity += 1;
        game_data.solidiers_cellar.push_back(solidier_type);

        content.game_data = game_data;
    });
}*/

void Siege::attackchoice(name player_name, name defender_name)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    eosio_assert(player_it.is_attacker == FALSE, "You are an attacker now!");

    _players.modify(player_it, player_name, [&](auto &content) {
        content.is_attacker = TRUE;
        content.opponent = defender_name;
    });
}

void Siege::defensechoice(name player_name, name attacker_name, uint64_t choice)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    auto &attacker_it = _players.get(attacker_name.value, "Cannot find the attacker!");

    //choice 0: leave the city
    if (choice == 0)
    {
        uint64_t city_id = player_it.own_city_id;
        // action(
        //     permission_level{player_name, N(active)},
        //     N(Siege), N(leaveCity),
        //     std::make_tuple(player_name)
        // ).send();
        //cities_table _cities(_self, _self.value);
        auto &city_it = _cities.get(city_id, "Cannot find the city!");
        double produced_bonus = city_it.produced_bonus;
        auto quantity = asset((produced_bonus + 0.01) * 10000, symbol("ZJUBCA", 4));

        action(
            permission_level{_self, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple(_self, player_name, quantity,
                            std::string("You have got the bonus and left the city!")))
            .send();

        //clear player's data
        _players.modify(player_it, player_name, [&](auto &content) {
            content.is_defender = FALSE;
            content.own_city_id = 0;
        });

        //modify city's data
        _cities.modify(city_it, player_name, [&](auto &content) {
            content.belong_player = attacker_name;
            content.produced_bonus = 0.0;
        });

        //modify attacker's data
        _players.modify(attacker_it, attacker_name, [&](auto &content) {
            content.is_attacker = FALSE;
            content.is_defender = TRUE;
            content.opponent = name(0);
            content.own_city_id = city_id;
        });
    }
    else
    {
        /* choice 1: defense */
        _players.modify(player_it, player_name, [&](auto &content) {
            content.opponent = attacker_name;
        });
    }
}

void Siege::picksolidier(name player_name, solidier solidier_type)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");

    _players.modify(player_it, player_name, [&](auto &content) {
        game_info game_data = content.game_data;

        eosio_assert(game_data.round_id <= 5, "This attack is over!");

        game_data.round_id += 1;
        int count = std::count(game_data.solidiers_cellar.begin(), game_data.solidiers_cellar.end(), solidier_type);
        eosio_assert(count != 0, "You don't have this type of solidier!");

        vector<solidier>::iterator it;
        it = std::find(game_data.solidiers_cellar.begin(), game_data.solidiers_cellar.end(), solidier_type);
        game_data.solidiers_cellar.erase(it);         //使用过的兵种不可以再使用，要从仓库中删除

        game_data.solidier_selected = solidier_type;
        content.game_data = game_data;
    });
}

void Siege::getresult(name player_name)      //赢家只负责更新自己的表，输家要更新城池的表
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    name opponent = player_it.opponent;
    auto &opponent_it = _players.get(opponent.value, "Cannot find your opponent!");
    
    uint64_t city_id = player_it.own_city_id || opponent_it.own_city_id;
    auto &city_it = _cities.get(city_id, "Cannot find the city!");
    //uint64_t current_city = city_it.city_id;

    eosio_assert(player_it.game_data.round_id == opponent_it.game_data.round_id, "Not in the same round!");

    //Get the result
    solidier my_solidier = player_it.game_data.solidier_selected;
    solidier opponent_solidier = opponent_it.game_data.solidier_selected;
    int8_t result = battleresult(my_solidier, opponent_solidier);

    if (result == 1 || result == 0)
    {
        /* do nothing */
        // _players.modify(player_it, player_name, [&](auto &content) {
        //     game_info game_data = content.game_data;

        //     game_data.solidier_selected = 0;

        //     content.game_data = game_data;
        // });
    }
    else if (result == -1)
    {
        _players.modify(player_it, player_name, [&](auto &content) {
            game_info game_data = content.game_data;

            double solidier_point = solidiers_point.at(game_data.solidier_selected);
            game_data.current_solidiers_point -= solidier_point;    //输掉一方扣除相应的士兵点数
            //game_data.solidier_selected = 0;

            content.game_data = game_data;
        });
    }
    else
    {
        /* Error */
    }

    if (player_it.game_data.round_id == 5)
    {
        point my_point = player_it.game_data.current_solidiers_point;
        point opponent_point = opponent_it.game_data.current_solidiers_point;
        
        name winner;
        _players.modify(player_it, player_name, [&](auto &content) {
            if (my_point < opponent_point)
            {
                content.if_lost = TRUE;
                content.if_win = FALSE;
            }
            else if (my_point > opponent_point)
            {
                content.if_lost = FALSE;
                content.if_win = TRUE;
            }
            else
            {
                /* do nothing */
                content.if_lost = FALSE;
                content.if_win = FALSE;
            }
        });

        if (player_it.is_defender == TRUE && player_it.is_attacker == FALSE && player_it.if_win == TRUE && player_it.if_lost == FALSE)
        {
            //clear player's data
            // _players.modify(player_it, player_name, [&](auto &content){
            //     content.opponent = 0;
            //     content.if_win = 0;
            // });
        }
        else if (player_it.is_defender == TRUE && player_it.is_attacker == FALSE && player_it.if_win == FALSE && player_it.if_lost == TRUE)
        {
            /* leave the city and loss my EOS */
            //uint64_t city_id = player_it.own_city_id;
            
            //cities_table _cities(_self, _self.value);
            //auto &city_it = _cities.get(city_id, "Cannot find the city!");
            double produced_bonus = city_it.produced_bonus;
            auto quantity = asset((produced_bonus + 0.01) * 10000, symbol("ZJUBCA", 4));

            action(
                permission_level{_self, "active"_n},
                "zjubca.token"_n, "transfer"_n,
                std::make_tuple(_self, player_name, quantity,
                                std::string("You have got the bonus and left the city!")))
                .send();          //take away the bonus

            //update player's data
            _players.modify(player_it, player_name, [&](auto &content) {
                content.is_defender = FALSE;
            });

            //modify city's data
            _cities.modify(city_it, player_name, [&](auto &content) {
                content.belong_player = opponent;
                content.produced_bonus = 0.01;
            });

            //transfer the EOS to the attacker
            auto lost_value = asset(player_it.game_data.all_solidiers_point * 5 * 10000, symbol("ZJUBCA", 4));

            action(
                permission_level{player_name, "active"_n},
                "zjubca.token"_n, "transfer"_n,
                std::make_tuple(_self, opponent, lost_value,
                                std::string("You got my EOS!")))
                .send();      //当初购买兵力的token储存在合约中，现在转给获胜者
        }
        else if (player_it.is_attacker == TRUE && player_it.is_defender == FALSE && player_it.if_win == TRUE && player_it.if_lost == FALSE)
        {
            //update player's data
            _players.modify(player_it, player_name, [&](auto &content) {
                content.is_attacker = FALSE;
                content.is_defender = TRUE;
                content.own_city_id = city_id;
            });
        }
        else if (player_it.is_attacker == TRUE && player_it.is_defender == FALSE && player_it.if_win == FALSE && player_it.if_lost == TRUE)
        {
            //update player's data
            _players.modify(player_it, player_name, [&](auto &content) {
                content.is_attacker = FALSE;
            });

            //transfer the EOS to the defender
            auto lost_value = asset(player_it.game_data.all_solidiers_point * 5 * 10000, symbol("ZJUBCA", 4));

            action(
                permission_level{player_name, "active"_n},
                "zjubca.token"_n, "transfer"_n,
                std::make_tuple(_self, opponent, lost_value,
                                std::string("You got my EOS!")))
                .send();
        }
        else
        {
            /* do nothing */
        }
    }
}

void Siege::endgame(name player_name)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    /* clear all the data */
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");

    _players.modify(player_it, player_name, [&](auto &content) {
        content.opponent = name(0);
        content.if_win = FALSE;
        content.if_lost = FALSE;

        game_info game_data;
        content.game_data = game_data;
        if (player_it.is_attacker == FALSE && player_it.is_defender == FALSE)
        {
            /* not a defender */
            content.own_city_id = 0;
        }
    });
}

/* transfer error */
void Siege::allend()
{
    //players_table _players(_self, _self.value);
    //cities_table _cities(_self, _self.value);
    //global_table _global(_self, _self.value);
    /* clear cities data */
    for (int city_id = 1; city_id <= CITY_NUM; city_id++)
    {
        auto &city_it = _cities.get(city_id, "Cannot find the city!");

        name own_name = city_it.belong_player;
        double produced_bonus = city_it.produced_bonus;
        if (own_name.value != 0 && produced_bonus != 0)
        {
            auto quantity = asset((produced_bonus + 0.01) * 10000, symbol("ZJUBCA", 4));

            action(
                permission_level{_self, "active"_n},
                "zjubca.token"_n, "transfer"_n,
                std::make_tuple(_self, own_name, quantity,
                                std::string("You have got the bonus and left the city!")))
                .send();
        }
    }

    /* delete the cities table */
    auto cities_begin_it = _cities.begin();
    while (cities_begin_it != _cities.end())
    {
        cities_begin_it = _cities.erase(cities_begin_it);
    }

    /* delete the global table */
    auto global_begin_it = _global.begin();
    while (global_begin_it != _global.end())
    {
        global_begin_it = _global.erase(global_begin_it);
    }

    /* delete the players table */
    auto players_begin_it = _players.begin();
    while (players_begin_it != _players.end())
    {
        players_begin_it = _players.erase(players_begin_it);
    }
}

EOSIO_DISPATCH(Siege, (allstart))

/*extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {
        if (code == receiver && action != "transfer"_n.value)
        {
            switch(action)
            {
            case "leavecity"_n.value:
                execute_action(name(receiver), name(code), &Siege::leavecity);
                break;
            case "attackchoice"_n.value:
                execute_action(name(receiver), name(code), &Siege::attackchoice);
                break;
            case "defensechoice"_n.value:
                execute_action(name(receiver), name(code), &Siege::defensechoice);
                break;
            case "picksolidier"_n.value:
                execute_action(name(receiver), name(code), &Siege::picksolidier);
                break;
            case "getresult"_n.value:
                execute_action(name(receiver), name(code), &Siege::getresult);
                break;
            case "endgame"_n.value:
                execute_action(name(receiver), name(code), &Siege::endgame);
                break;
            case "allend"_n.value:
                execute_action(name(receiver), name(code), &Siege::allend);
                break;
            default:
                break;
            }
        }
        else if (code == "zjubca.token"_n.value && action == "transfer"_n.value)
        {
            execute_action(name(receiver), name(receiver), &Siege::transfer);
        }
    }
}*/




