#include "eossiege.hpp"
#include "function.cpp"


void EOSSiege::allstart()
{
    /* after this action, then all the other actions can be act */
    // 只有本合约才能执行此action
    require_auth(_self);
    // 初始化城池信息
    for (uint64_t i = 1; i <= CITY_NUM; i++)
    {
        auto city_it = _cities.find(i);
        if (city_it == _cities.end())
        {
            city_it = _cities.emplace(_self, [&](auto &new_city) {
                new_city.city_id = i;
                new_city.city_name = city_name_map.at(i);
                new_city.defense_index = city_defense_index_map.at(i);
            });
        }
    }
    // 初始化全局信息
    auto global_it = _global.find(1);
    if (global_it == _global.end())
    {
        global_it = _global.emplace(_self, [&](auto &new_global) {
          new_global.produce_rate = 0.0;
        });
    }
}

void EOSSiege::transfer(name player_name, name to, asset quantity, string memo)
{
    vector<string> info;
    split(memo, ',', info);

    // 缴纳入场费进入游戏
    if (!strcmp(info[0].c_str(), "StartSiege"))
    {
        // 一些assert
        require_auth(player_name);
        // 确保游戏处于starting阶段
        auto &global_it = _global.get(1, "get global table failed");
        eosio_assert(global_it.game_stage == 1, "game is not in starting stage");
        
        eosio_assert(player_name != to, "cannot transfer to self");
        eosio_assert(to == _self, "only can transfer the contract");
        
        auto sym = quantity.symbol;
        eosio_assert(sym.is_valid(), "invalid symbol name");
        eosio_assert(sym == symbol("EOS", 4), "invalid symbol name");
        
        eosio_assert(quantity.is_valid(), "invalid quantity");
        eosio_assert(quantity.amount == ENTER_FEE, "must transfer 0.5 EOS to enter the game");
        
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
        
        // 向players表格中插入用户信息
        auto existing = _players.find(player_name.value);
        if (existing == _players.end())
        {
            existing = _players.emplace(_self, [&](auto &new_player) {
                new_player.player = player_name;
            });
        }

        auto &player_it = _players.get(player_name.value, "Cannot find the player!");
        _players.modify(player_it, same_payer, [&](auto &content) {
            game_info game_data;

            //initialize the game data for players
            content.game_data = game_data;
        });
    }
    // 缴纳拍卖尾款占领城池
    else if (!strcmp(info[0].c_str(), "SiegeOccupation1"))
    {
        require_auth(player_name);
        // 确保游戏处于bidding阶段
        auto &global_it = _global.get(1, "get global table failed");
        eosio_assert(global_it.game_stage == 2, "game is not in bidding stage");
        
        eosio_assert(player_name != to, "cannot transfer to self");
        eosio_assert(to == _self, "only can transfer the contract");
        // 为了防止玩家恶意占领城池，加入身份验证信息，判断其是否在拍卖表中
        auto rank_it = _rank.get(player_name.value, "you are not in the bidding player list");
        
        auto sym = quantity.symbol;
        eosio_assert(sym.is_valid(), "invalid symbol name");
        eosio_assert(sym == symbol("EOS", 4), "invalid symbol name");
        
        eosio_assert(quantity.is_valid(), "invalid quantity");
        // 确保用户缴纳数额正确
        eosio_assert(quantity == rank_it.bidding_price, "bidding_price not match");
        
        uint64_t city_idx = stoull(info[1].c_str());
        auto &player_it = _players.get(player_name.value, "Cannot find the player!");
        eosio_assert(player_it.is_attacker == 0, "You now are an attacker!");
        eosio_assert(player_it.is_defender == 0, "You have already occupied another city!");
        
        _players.modify(player_it, same_payer, [&](auto &content) {
            content.is_defender = 1;
            content.own_city_id = city_idx;
        });
        
        auto &city_it = _cities.get(city_idx, "Cannot find the city!");
        eosio_assert(city_it.if_be_occupied == 0, "The city has been occupied before.");
        
        _cities.modify(city_it, same_payer, [&](auto &content) {
            content.if_be_occupied = 1;
            content.belong_player = player_name;
        });
        
        eosio_assert(global_it.cities_remain > 0, "No city remained!");
        
        _global.modify(global_it, same_payer, [&](auto &content) {
            content.cities_remain -= 1;
            content.produce_rate *= (CITY_NUM - content.cities_remain - 1) / (CITY_NUM - content.cities_remain);
        });
    }
    //else if(info[0].c_str() == "SiegeBuysoldiers")
    else if (!strcmp(info[0].c_str(), "SiegeBuySoldiers"))
    {
        require_auth(player_name);
        eosio_assert(player_name != to, "cannot transfer to self");
        eosio_assert(to == _self, "only can transfer the contract");
        
        auto sym = quantity.symbol;
        eosio_assert(sym.is_valid(), "invalid symbol name");
        eosio_assert(sym == symbol("EOS", 4), "invalid symbol name");
        
        eosio_assert(quantity.is_valid(), "invalid quantity");
        
        soldier soldier_type = stoull(info[2].c_str());
        double soldier_point = soldiers_point.at(soldier_type);
        double soldier_price = soldier_point;    //价格等于点数的5倍，可调整
        eosio_assert(quantity.amount == soldier_price * 10000, "price is not match");
        //players_table _players(_self, _self.value);
        auto &player_it = _players.get(player_name.value, "Cannot find the player!");
        eosio_assert(player_it.game_data.status == RUNNING, "Game is over!");
        eosio_assert(player_it.game_data.all_soldiers_point + soldier_point <= 4.0,
                    "You soldier point is max!");
        // eosio_assert(player_it.game_data.soldier_quantity < 5,
        //             "You soldier quantity is max!");

        _players.modify(player_it, same_payer, [&](auto &content) {
            game_info game_data = content.game_data;

            //update the game info
            game_data.all_soldiers_point += soldier_point;
            game_data.current_soldiers_point += soldier_point;
            game_data.soldier_quantity += 1;
            game_data.soldiers_cellar.push_back(soldier_type);

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

void EOSSiege::updateranktb(uint64_t ranking, name player, asset bidding_price, string bidding_time)
{
  require_auth(_self);
  eosio_assert(ranking >= 1, "ranking is out of range");
  eosio_assert(ranking <= 25, "ranking is out of range");
  auto sym = bidding_price.symbol;
  eosio_assert(sym.is_valid(), "invalid symbol name");
  eosio_assert(sym == symbol("EOS", 4), "invalid symbol name");
  eosio_assert(bidding_price.is_valid(), "invalid bidding_price");
  
  auto rank_it = _rank.find(player.value);
  if(rank_it == _rank.end())
  {
    rank_it = _rank.emplace(_self, [&](auto &new_rank){
      new_rank.ranking = ranking;
      new_rank.player = player;
      new_rank.bidding_price = bidding_price;
      new_rank.bidding_time = bidding_time;
    });
  }
  else
  {
    auto &rank_it = _rank.get(ranking, "Cannot find the ranking!");
    
    _rank.modify(rank_it, same_payer, [&](auto content){
      content.player = player;
      content.bidding_price = bidding_price;
      content.bidding_time = bidding_time;
    });
  }
}

void EOSSiege::updatestage(uint64_t stage)
{
  require_auth(_self);
  eosio_assert((stage == 1) || (stage == 2) || (stage == 3) || (stage == 4), "game stage error");
  
  auto &global_it = _global.get(1, "get global table failed");
  _global.modify(global_it, same_payer, [&](auto &content) {
    content.game_stage = stage;
  });
}

void EOSSiege::departure(name player_name)
{
    //Authrozied
    require_auth(player_name);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    
    _players.modify(player_it, same_payer, [&](auto &content) {
      game_info game_data = content.game_data;
      game_data.prepare_ok = TRUE;
      content.game_data = game_data;
    });
}

void EOSSiege::leavecity(name player_name)
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
    auto quantity = asset(produced_bonus, symbol("EOS", 4));     //此处bonus仍然是固定的

    action(
        permission_level{_self, "active"_n},
        "eosio.token"_n, "transfer"_n,
        std::make_tuple(_self, player_name, quantity,
                        std::string("You have got the bonus and left the city!")))
        .send();
    

    //clear data
    _players.modify(player_it, same_payer, [&](auto &content) {
        content.is_defender = FALSE;
        content.own_city_id = 0;
    });

    _cities.modify(city_it, same_payer, [&](auto &content) {
        content.if_be_occupied = FALSE;
        content.belong_player = name(0);
        content.produced_bonus = 0;
    });

    //global_table _global(_self, _self.value);
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.cities_remain < 5, "All the city is not occupied!");

    _global.modify(global_it, same_payer, [&](auto &content) {
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

void EOSSiege::attack(name player_name, name defender_name)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    eosio_assert(player_it.is_attacker == FALSE, "You are an attacker now!");
    
    auto &defender_it = _players.get(defender_name.value, "Cannot find your target!");
    eosio_assert(defender_it.be_attacked_request == FALSE, "Your target is under attacked!");

    _players.modify(player_it, same_payer, [&](auto &content) {
        content.is_attacker = TRUE;
        content.opponent = defender_name;
    });
    
    _players.modify(defender_it, same_payer, [&](auto &content) {
        content.be_attacked_request = TRUE;
        content.opponent = player_name;
    });
}

void EOSSiege::defense(name player_name, name attacker_name, uint64_t choice)
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
        auto quantity = asset(produced_bonus, symbol("EOS", 4));

        action(
            permission_level{_self, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple(_self, player_name, quantity,
                            std::string("You have got the bonus and left the city!")))
            .send();

        //clear player's data
        _players.modify(player_it, same_payer, [&](auto &content) {
            content.is_defender = FALSE;
            content.be_attacked_request = FALSE;
            content.opponent = name(0);
            content.own_city_id = 0;
        });

        //modify city's data
        _cities.modify(city_it, same_payer, [&](auto &content) {
            content.belong_player = attacker_name;
            content.produced_bonus = 0;
        });

        //modify attacker's data
        _players.modify(attacker_it, same_payer, [&](auto &content) {
            content.is_attacker = FALSE;
            content.is_defender = TRUE;
            content.opponent = name(0);
            content.own_city_id = city_id;
        });
    }
    else
    {
        //choice 1: defense 
        _players.modify(player_it, same_payer, [&](auto &content) {
            content.be_attacked_request = FALSE;
            content.in_battle = TRUE;
        });
        
        _players.modify(attacker_it, same_payer, [&](auto &content) {
            content.in_battle = TRUE;
        });
    }
}

void EOSSiege::picksoldier(name player_name, soldier soldier_type)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");

    _players.modify(player_it, same_payer, [&](auto &content) {
        game_info game_data = content.game_data;

        eosio_assert(game_data.round_id < 5, "This attack is over!");    //changed
        eosio_assert(game_data.is_round_over == TRUE, "Last round is not over");

        game_data.round_id += 1;
        int count = std::count(game_data.soldiers_cellar.begin(), game_data.soldiers_cellar.end(), soldier_type);
        eosio_assert(count != 0, "You don't have this type of soldier!");

        vector<soldier>::iterator it;
        it = std::find(game_data.soldiers_cellar.begin(), game_data.soldiers_cellar.end(), soldier_type);
        game_data.soldiers_cellar.erase(it);         //使用过的兵种不可以再使用，要从仓库中删除

        game_data.soldier_selected = soldier_type;
        game_data.is_round_over = FALSE;
        content.game_data = game_data;
    });
}

void EOSSiege::getresult(name player_name)      //赢家只负责更新自己的表，输家要更新城池的表
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    name opponent = player_it.opponent;
    auto &opponent_it = _players.get(opponent.value, "Cannot find your opponent!");
    
    uint64_t city_id = player_it.own_city_id == 0 ? opponent_it.own_city_id : player_it.own_city_id;
    //print(city_id);
    auto &city_it = _cities.get(city_id, "Cannot find the city!");
    //uint64_t current_city = city_it.city_id;

    eosio_assert(player_it.game_data.round_id == opponent_it.game_data.round_id, "Not in the same round!");
    eosio_assert(player_it.game_data.is_round_over == FALSE, "this round is over");

    //Get the result
    soldier my_soldier = player_it.game_data.soldier_selected;
    soldier opponent_soldier = opponent_it.game_data.soldier_selected;
    int8_t result = battleresult(my_soldier, opponent_soldier);

    if (result == 1 || result == 0)
    {
        /* do nothing */
        // _players.modify(player_it, player_name, [&](auto &content) {
        //     game_info game_data = content.game_data;

        //     game_data.soldier_selected = 0;

        //     content.game_data = game_data;
        // });
        _players.modify(player_it, same_payer, [&](auto &content) {
            game_info game_data = content.game_data;
            game_data.is_round_over = TRUE;
            game_data.soldier_selected = 0;
            content.game_data = game_data;
        });
    }
    else if (result == -1)
    {
        _players.modify(player_it, same_payer, [&](auto &content) {
            game_info game_data = content.game_data;

            double soldier_point = soldiers_point.at(game_data.soldier_selected);
            game_data.current_soldiers_point -= soldier_point;    //输掉一方扣除相应的士兵点数
            game_data.soldier_selected = 0;
            game_data.is_round_over = TRUE;

            content.game_data = game_data;
        });
    }
    else
    {
        /* Error */
    }

    if (player_it.game_data.round_id == 5)
    {
        point my_point = player_it.game_data.current_soldiers_point;
        point opponent_point = opponent_it.game_data.current_soldiers_point;
        print(my_point, opponent_point);
        
        //name winner;
        _players.modify(player_it, same_payer, [&](auto &content) {
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
            auto quantity = asset(produced_bonus, symbol("EOS", 4));

            action(
                permission_level{_self, "active"_n},
                "eosio.token"_n, "transfer"_n,
                std::make_tuple(_self, player_name, quantity,
                                std::string("You have got the bonus and left the city!")))
                .send();          //take away the bonus

            //update player's data
            _players.modify(player_it, same_payer, [&](auto &content) {
                content.is_defender = FALSE;
                //content.own_city_id = 0;
            });

            //modify city's data
            _cities.modify(city_it, same_payer, [&](auto &content) {
                content.belong_player = opponent;
                content.produced_bonus = 0;
            });

            //transfer the EOS to the attacker
            auto lost_value = asset(player_it.game_data.all_soldiers_point, symbol("EOS", 4));

            action(
                permission_level{_self, "active"_n},
                "eosio.token"_n, "transfer"_n,
                std::make_tuple(_self, opponent, lost_value,
                                std::string("You got my EOS!")))
                .send();      //当初购买兵力的token储存在合约中，现在转给获胜者
        }
        else if (player_it.is_attacker == TRUE && player_it.is_defender == FALSE && player_it.if_win == TRUE && player_it.if_lost == FALSE)
        {
            //update player's data
            _players.modify(player_it, same_payer, [&](auto &content) {
                content.is_attacker = FALSE;
                content.is_defender = TRUE;
                content.own_city_id = city_id;
            });
        }
        else if (player_it.is_attacker == TRUE && player_it.is_defender == FALSE && player_it.if_win == FALSE && player_it.if_lost == TRUE)
        {
            //update player's data
            _players.modify(player_it, same_payer, [&](auto &content) {
                content.is_attacker = FALSE;
            });

            //transfer the EOS to the defender
            auto lost_value = asset(player_it.game_data.all_soldiers_point, symbol("EOS", 4));

            action(
                permission_level{_self, "active"_n},
                "eosio.token"_n, "transfer"_n,
                std::make_tuple(_self, opponent, lost_value,
                                std::string("You got my EOS!")))
                .send();
        }
        else
        {
            /* do nothing */
            auto back_value = asset(player_it.game_data.all_soldiers_point, symbol("EOS", 4));
            
            action(
                permission_level{_self, "active"_n},
                "eosio.token"_n, "transfer"_n,
                std::make_tuple(_self, player_name, back_value,
                                std::string("Return your token!")))
                .send();
        }
    }
}

void EOSSiege::endgame(name player_name)
{
    //Authrozied
    require_auth(player_name);

    //players_table _players(_self, _self.value);
    // clear all the data
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");

    _players.modify(player_it, same_payer, [&](auto &content) {
        content.opponent = name(0);
        content.if_win = FALSE;
        content.if_lost = FALSE;

        game_info game_data;
        content.game_data = game_data;
        if (player_it.is_attacker == FALSE && player_it.is_defender == FALSE)
        {
            //not a defender
            content.own_city_id = 0;
        }
    });
}

/* transfer error */
void EOSSiege::allend()
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
            auto quantity = asset(produced_bonus, symbol("EOS", 4));

            action(
                permission_level{_self, "active"_n},
                "eosio.token"_n, "transfer"_n,
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
    
    /*delete the bidding ranking table */
    auto rank_begin_it = _rank.begin();
    while (rank_begin_it != _rank.end())
    {
      rank_begin_it = _rank.erase(rank_begin_it);
    }
}


//EOSIO_DISPATCH(Siege, (allstart)(transfer)(leavecity)(attack)(defense)(picksoldier)(getresult)(endgame)(allend))

extern "C" {
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {
        if (code == receiver && action != "transfer"_n.value)
        {
            switch(action)
            {
            case "allstart"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::allstart);
                break;
            case "updateranktb"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::updateranktb);
                break;
            case "updatestage"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::updatestage);
                break;
            case "departure"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::departure);
                break;
            case "leavecity"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::leavecity);
                break;
            case "attack"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::attack);
                break;
            case "defense"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::defense);
                break;
            case "picksoldier"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::picksoldier);
                break;
            case "getresult"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::getresult);
                break;
            case "endgame"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::endgame);
                break;
            case "allend"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::allend);
                break;
            default:
                break;
            }
        }
        else if (code == "eosio.token"_n.value && action == "transfer"_n.value)
        {
            execute_action(name(receiver), name(receiver), &EOSSiege::transfer);
        }
    }
}




