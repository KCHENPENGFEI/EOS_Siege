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
          new_global.game_stage = START;
          new_global.produce_rate = 0.0;
        });
    }
    else
    {
        auto &global_it = _global.get(1, "get global table failed");
        _global.modify(global_it, same_payer, [&](auto &content){
          content.cities_remain = CITY_NUM;
          content.game_stage = START;
          content.produce_rate = 0.0;
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
        
        // 确保玩家不在冻结名单
        eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
        
        // 确保游戏处于starting阶段
        auto &global_it = _global.get(1, "get global table failed");
        eosio_assert(global_it.game_stage == START, "game is not in starting stage");
        
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
        // 确保玩家不在冻结名单
        eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
        
        // 确保游戏处于bidding阶段
        auto &global_it = _global.get(1, "get global table failed");
        eosio_assert(global_it.game_stage == BIDDING, "game is not in bidding stage");
        
        eosio_assert(player_name != to, "cannot transfer to self");
        eosio_assert(to == _self, "only can transfer the contract");
        // 为了防止玩家恶意占领城池，加入身份验证信息，判断其是否在拍卖表中
        auto rank_it = _rank.get(player_name.value, "you are not in the bidding player list");
        
        auto sym = quantity.symbol;
        eosio_assert(sym.is_valid(), "invalid symbol name");
        eosio_assert(sym == symbol("EOS", 4), "invalid symbol name");
        
        eosio_assert(quantity.is_valid(), "invalid quantity");
        // 确保玩家缴纳数额正确
        eosio_assert(quantity == rank_it.bidding_price, "bidding_price not match");
        
        // 拍卖后的城池分配不支持玩家操作，只能后台调用合约进行分配
        
        // uint64_t city_idx = stoull(info[1].c_str());
        // auto &player_it = _players.get(player_name.value, "Cannot find the player!");
        // eosio_assert(player_it.is_attacker == 0, "You now are an attacker!");
        // eosio_assert(player_it.is_defender == 0, "You have already occupied another city!");
        
        // _players.modify(player_it, same_payer, [&](auto &content) {
        //     content.is_defender = 1;
        //     content.own_city_id = city_idx;
        // });
        
        // auto &city_it = _cities.get(city_idx, "Cannot find the city!");
        // eosio_assert(city_it.if_be_occupied == 0, "The city has been occupied before.");
        
        // _cities.modify(city_it, same_payer, [&](auto &content) {
        //     content.realtime_price = quantity;
        //     content.if_be_occupied = 1;
        //     content.belong_player = player_name;
        // });
        
        // eosio_assert(global_it.cities_remain > 0, "No city remained!");
        
        // _global.modify(global_it, same_payer, [&](auto &content) {
        //     content.cities_remain -= 1;
        //     content.produce_rate *= (CITY_NUM - content.cities_remain - 1) / (CITY_NUM - content.cities_remain);
        // });
    }
    // 游戏中玩家占领空闲城池
    else if (!strcmp(info[0].c_str(), "SiegeOccupation2"))
    {
      require_auth(player_name);
      // 确保玩家不在冻结名单
      eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
        
      // 确保游戏处于running阶段
      auto &global_it = _global.get(1, "get global table failed");
      eosio_assert(global_it.game_stage == RUNNING, "game is not in running stage");
      // 确保还有剩余城池
      eosio_assert(global_it.cities_remain > 0, "No city remained!");
      
      eosio_assert(player_name != to, "cannot transfer to self");
      eosio_assert(to == _self, "only can transfer the contract");
      
      auto sym = quantity.symbol;
      eosio_assert(sym.is_valid(), "invalid symbol name");
      eosio_assert(sym == symbol("EOS", 4), "invalid symbol name");
        
      eosio_assert(quantity.is_valid(), "invalid quantity");
      
      // 确保玩家为自由人
      auto &player_it = _players.get(player_name.value, "Cannot find the player!");
      eosio_assert(player_it.is_attacker == 0, "You now are an attacker!");
      eosio_assert(player_it.is_defender == 0, "You have already occupied another city!");
      
      uint64_t city_idx = stoull(info[1].c_str());
      auto &city_it = _cities.get(city_idx, "Cannot find the city!");
      eosio_assert(city_it.if_be_occupied == 0, "The city has been occupied before.");
      // 确保玩家缴纳数额正确
      eosio_assert(city_it.realtime_price == quantity, "price not match");
      
      _players.modify(player_it, same_payer, [&](auto &content) {
          content.is_defender = 1;
          content.own_city_id = city_idx;
      });

      _cities.modify(city_it, same_payer, [&](auto &content) {
          content.if_be_occupied = 1;
          content.belong_player = player_name;
      });

      _global.modify(global_it, same_payer, [&](auto &content) {
          content.cities_remain -= 1;
          content.produce_rate *= (CITY_NUM - content.cities_remain - 1) / (CITY_NUM - content.cities_remain);
      });
    }
    // 玩家购买士兵
    else if (!strcmp(info[0].c_str(), "SiegeBuySoldiers"))
    {
        require_auth(player_name);
        // 确保玩家不在冻结名单
        eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
        
        // 确保游戏处于running阶段
        auto &global_it = _global.get(1, "get global table failed");
        eosio_assert(global_it.game_stage == RUNNING, "game is not in running stage");
        
        // 验证玩家信息
        eosio_assert(player_name != to, "cannot transfer to self");
        eosio_assert(to == _self, "only can transfer the contract");
        
        // 验证转账是否为eos
        auto sym = quantity.symbol;
        eosio_assert(sym.is_valid(), "invalid symbol name");
        eosio_assert(sym == symbol("EOS", 4), "invalid symbol name");
        eosio_assert(quantity.is_valid(), "invalid quantity");
        
        // 验证选择的士兵和出价是否匹配
        string all_soldiers = info[2].c_str();
        vector<string> soldiers;
        split(all_soldiers, ';', soldiers);
        int i = 0;
        point all_soldiers_point = 0;
        while(soldiers[i] != "")
        {
          all_soldiers_point += soldiers_point.at(stoull(soldiers[i].c_str()));
          i++;
        }
        eosio_assert(quantity.amount == all_soldiers_point, "price is not match");
        
        // 确保玩家战力不超过10000
        eosio_assert(all_soldiers_point <= 10000, "You soldier point is max!");
        
        // soldier soldier_type = stoull(info[2].c_str());
        // double soldier_point = soldiers_point.at(soldier_type);
        // double soldier_price = soldier_point;    //价格等于点数的5倍，可调整
        // eosio_assert(quantity.amount == soldier_price * 10000, "price is not match");
        //players_table _players(_self, _self.value);
        // 确保玩家存在
        auto &player_it = _players.get(player_name.value, "Cannot find the player!");
        
        // 确保玩家处于备战(before_battle)状态
        eosio_assert(player_it.before_battle == TRUE, "You are not in preparing stage");
        // eosio_assert(player_it.game_data.status == RUNNING, "Game is over!");
        // eosio_assert(player_it.game_data.all_soldiers_point + soldier_point <= 4.0,
        //             "You soldier point is max!");
        // eosio_assert(player_it.game_data.soldier_quantity < 5,
        //             "You soldier quantity is max!");

        _players.modify(player_it, same_payer, [&](auto &content) {
            game_info game_data = content.game_data;

            //update the game info
            game_data.all_soldiers_point = all_soldiers_point;
            game_data.current_soldiers_point = all_soldiers_point;
            game_data.soldier_quantity = i;
            while(i)
            {
              game_data.soldiers_cellar.push_back(stoull(soldiers[5 - i].c_str()));
              i--;
            }
            content.game_data = game_data;
        });
        
        // 为了防止出现平局退款，而奖金池不足的情况，将玩家购买士兵的eos存入临时账户，和奖池分离
        action(
            permission_level{_self, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple(_self, "temptreasure"_n, quantity,
                            std::string("buysoldiers")))
            .send();
    }
    

    // action(
    //     permission_level{player_name, "active"_n},
    //     "eosio.token"_n, "transfer"_n,
    //     std::make_tuple(player_name, _self, quantity,
    //                     std::string("Enter game!")))
    //     .send();
}

void EOSSiege::allocatecity(name player_name, uint64_t city_id, asset price)
{
  require_auth(_self);
  
  // 确保玩家不在冻结名单
  eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
  
  auto &player_it = _players.get(player_name.value, "Cannot find the player!");
  auto &city_it = _cities.get(city_id, "Cannot find the city!");
  auto &global_it = _global.get(1, "get global table failed");
  auto &rank_it = _rank.get(player_name.value, "you are not in the bidding player list");
  
  eosio_assert(player_it.is_attacker == FALSE, "You now are an attacker!");
  eosio_assert(player_it.is_defender == FALSE, "You have already occupied another city!");
  eosio_assert(city_it.if_be_occupied == FALSE, "The city has been occupied before.");
  eosio_assert(global_it.game_stage == BIDDING, "game is not in bidding stage");
  eosio_assert(global_it.cities_remain > 0, "No city remained!");
  eosio_assert(rank_it.bidding_price == price, "bidding price is not match");
  
  // 更新玩家表
  _players.modify(player_it, same_payer, [&](auto &content) {
    content.is_defender = TRUE;
    content.own_city_id = city_id;
  });
  
  // 更新城池表
  _cities.modify(city_it, same_payer, [&](auto &content) {
    content.realtime_price = price;
    content.if_be_occupied = TRUE;
    content.belong_player = player_name;
  });
  
  // 更新全局表
  _global.modify(global_it, same_payer, [&](auto &content) {
    content.cities_remain -= 1;
    content.produce_rate *= (CITY_NUM - content.cities_remain - 1) / (CITY_NUM - content.cities_remain);
  });
  
}

void EOSSiege::freezeplayer(name player_name, uint64_t rank, string frozen_time)
{
  require_auth(_self);
  // 确保玩家不在冻结名单
  eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
  
  // 3种冻结等级
  eosio_assert((rank == 1) || (rank == 2) || (rank == 3), "freeze rank error");
  
  auto frozen_it = _frozen.find(player_name.value);
  eosio_assert(frozen_it == _frozen.end(), "player is in the frozen list");
  frozen_it = _frozen.emplace(_self, [&](auto &new_frozen){
    new_frozen.player = player_name;
    new_frozen.frozen_rank = rank;
    new_frozen.frozen_time = frozen_time;
  });
}

void EOSSiege::updateranktb(uint64_t ranking, name player_name, asset bidding_price, string bidding_time)
{
  require_auth(_self);
  // 确保玩家不在冻结名单
  eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
  
  eosio_assert(ranking >= 1, "ranking is out of range");
  eosio_assert(ranking <= CITY_NUM, "ranking is out of range");
  
  auto sym = bidding_price.symbol;
  eosio_assert(sym.is_valid(), "invalid symbol name");
  eosio_assert(sym == symbol("EOS", 4), "invalid symbol name");
  eosio_assert(bidding_price.is_valid(), "invalid bidding_price");
  
  auto &global_it = _global.get(1, "get global table failed");
  eosio_assert(global_it.game_stage == BIDDING, "game is not in bidding stage");
  
  auto rank_it = _rank.find(player_name.value);
  if(rank_it == _rank.end())
  {
    rank_it = _rank.emplace(_self, [&](auto &new_rank){
      new_rank.ranking = ranking;
      new_rank.player = player_name;
      new_rank.bidding_price = bidding_price;
      new_rank.bidding_time = bidding_time;
    });
  }
  else
  {
    auto &rank_it = _rank.get(ranking, "Cannot find the ranking!");
    
    _rank.modify(rank_it, same_payer, [&](auto content){
      content.player = player_name;
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
    // require_auth(player_name);
    require_auth(_self);
    // 确保玩家不在冻结名单
    eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
    
    auto &global_it = _global.get(1, "get global table failed");
    eosio_assert(global_it.game_stage == RUNNING, "game is not in running stage");
    
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    eosio_assert(player_it.before_battle == TRUE, "You are not in  any battle");
    eosio_assert(player_it.in_battle == FALSE, "You are in a battle now");
    
    _players.modify(player_it, same_payer, [&](auto &content) {
      // 状态转换至in_battle
      content.before_battle = FALSE;
      content.in_battle = TRUE;
      
      // 将round_id至为1
      game_info game_data = content.game_data;
      game_data.round_id = 1;
      content.game_data = game_data;
    });
}

void EOSSiege::leavecity(name player_name)
{
    //Authrozied
    require_auth(player_name);
    // 确保玩家不在冻结名单
    eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
    
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.game_stage == RUNNING, "game is not in running stage");
    eosio_assert(global_it.cities_remain < CITY_NUM, "All the city is not occupied!");
    
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    // 确保玩家拥有城池
    eosio_assert(player_it.is_defender == TRUE, "You don't have any city!");

    uint64_t city_id = player_it.own_city_id;

    //withdraw the produced bonus
    //cities_table _cities(_self, _self.value);
    auto &city_it = _cities.get(city_id, "Cannot find the city!");
    // 确保城池为玩家所有
    eosio_assert(city_it.belong_player == player_name, "You don't have any city!");
    uint64_t produced_bonus = city_it.produced_bonus;
    auto quantity = asset(produced_bonus, symbol("EOS", 4));     //此处bonus仍然是固定的
    
    // 合约对外转账
    if(quantity.amount != 0)
    {
      action(
        permission_level{_self, "active"_n},
        "eosio.token"_n, "transfer"_n,
        std::make_tuple(_self, player_name, quantity,
                        std::string("You have got the bonus and left the city!")))
        .send();
    }

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

    _global.modify(global_it, same_payer, [&](auto &content) {
        content.cities_remain += 1;
        // 所有城池均为空，此时出产率为0
        if (content.cities_remain == CITY_NUM)
        {
            content.produce_rate = 0;
        }
        else
        {
            // 出产率由于玩家弃城而提高
            content.produce_rate *= (CITY_NUM - content.cities_remain + 1) / (CITY_NUM - content.cities_remain);
        }
    });
}

void EOSSiege::attack(name player_name, name defender_name)
{
    //Authrozied
    // require_auth(player_name);
    require_auth(_self);   // 改成了合约才能操作
    // 确保玩家不在冻结名单
    eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
    
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.game_stage == RUNNING, "game is not in running stage");
    
    // 确保玩家为游民
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    eosio_assert(player_it.is_attacker == FALSE, "You are an attacker now!");
    eosio_assert(player_it.is_defender == FALSE, "You are a defender now");
    
    auto &defender_it = _players.get(defender_name.value, "Cannot find your target!");
    // 确保玩家为防守者，并且不处于被攻击状态
    eosio_assert(defender_it.is_defender == TRUE, "You target is not a defender!");
    eosio_assert(defender_it.be_attacked_request == FALSE, "Your target is under attacked!");
    eosio_assert(defender_it.before_battle == FALSE, "Your target is under attacked!");
    eosio_assert(defender_it.in_battle == FALSE, "Your target is under attacked!");
    
    // 标记成为攻击者，可以防止玩家同时攻击多个目标
    _players.modify(player_it, same_payer, [&](auto &content) {
        content.is_attacker = TRUE;
        content.opponent = defender_name;
    });
    
    // 标记为被攻击者，可以防止玩家同时被多个进攻者攻击
    _players.modify(defender_it, same_payer, [&](auto &content) {
        content.be_attacked_request = TRUE;
        content.opponent = player_name;
    });
}

void EOSSiege::temp()
{
  require_auth(_self);
  
  
  // for(int i = 1; i<= 25; i++)
  // {
  //   auto &it = _cities.get(i, "error");
  //   _cities.modify(it, same_payer, [&](auto &content){
  //     content.produced_bonus = 100;
  //   });
  // }
  // auto quantity = asset(10000, symbol("EOS", 4));
  // action(
  //   permission_level{"temptreasure"_n, "active"_n},
  //   "eosio.token"_n, "transfer"_n,
  //   std::make_tuple("temptreasure"_n, _self, quantity,
  //                   std::string("transfer success")))
  //   .send();
  auto &it1 = _players.get("player1"_n.value, "E");
  _players.modify(it1, same_payer, [&](auto &content){
    content.before_battle = TRUE;
    content.in_battle = FALSE;
  });
  
  auto &it2 = _players.get("attack1"_n.value, "E");
  _players.modify(it2, same_payer, [&](auto &content){
    content.before_battle = TRUE;
    content.in_battle = FALSE;
  });
}

void EOSSiege::defense(name player_name, name attacker_name, uint64_t city_id, uint64_t choice)
{
    //Authrozied
    // require_auth(player_name);
    require_auth(_self);
    
    // 确保玩家不在冻结名单
    eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
    
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.game_stage == RUNNING, "game is not in running stage");
    
    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    auto &attacker_it = _players.get(attacker_name.value, "Cannot find the attacker!");
    eosio_assert(player_it.own_city_id == city_id, "You don't have this city");
    auto &city_it = _cities.get(city_id, "Cannot find the city!");

    //choice 0: leave the city
    if (choice == 0)
    {
        // uint64_t city_id = player_it.own_city_id;
        // action(
        //     permission_level{player_name, N(active)},
        //     N(Siege), N(leaveCity),
        //     std::make_tuple(player_name)
        // ).send();
        //cities_table _cities(_self, _self.value);
        
        uint64_t produced_bonus = city_it.produced_bonus;
        auto quantity = asset(produced_bonus, symbol("EOS", 4));
        
        if(quantity.amount != 0)
        {
          action(
            permission_level{_self, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple(_self, player_name, quantity,
                            std::string("You have got the bonus and left the city!")))
            .send();
        }

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
      
      // 使用inline_action调用leavecity(inline_action在本次action之后才能执行)
      // action(
      //   permission_level{player_name, "active"_n},
      //   _self, "leavecity"_n,
      //   std::make_tuple(player_name)).send();
      
      // clear player's data
      // _players.modify(player_it, same_payer, [&](auto &content) {
      //   content.be_attacked_request = FALSE;
      //   content.opponent = name(0);
      // });
      
      // modify attacker's data 可以考虑修改，尽量保证只有玩家自己(或者合约)才可以修改属于自己的表
      // _players.modify(attacker_it, same_payer, [&](auto &content) {
      //   content.is_attacker = FALSE;
      //   content.is_defender = TRUE;
      //   content.opponent = name(0);
      //   content.own_city_id = city_id;
      // })
    }
    else if(choice == 1)
    {
        //choice 1: defense 
        _players.modify(player_it, same_payer, [&](auto &content) {
          // 状态切换至before_battle
          content.be_attacked_request = FALSE;
          content.before_battle = TRUE;
        });
        
        _players.modify(attacker_it, same_payer, [&](auto &content) {
          // 状态切换至before_battle
          content.before_battle = TRUE;
        });
    }
    else
    {
      // Error
    }
}

void EOSSiege::picksoldier(name player_name, soldier soldier_type)
{
    //Authrozied
    // require_auth(player_name);
    require_auth(_self);
    
    // 确保玩家不在冻结名单
    eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
    
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.game_stage == RUNNING, "game is not in running stage");
    
    //players_table _players(_self, _self.value);
    auto &player_it = _players.get(player_name.value, "Cannot find the player!");
    // 确保玩家处在in_battle阶段
    eosio_assert(player_it.in_battle == TRUE, "You are not in any battle");
    
    // 确保round <= 5，且上一轮已经结束
    eosio_assert(player_it.game_data.round_id <= 5, "This attack is over!");
    eosio_assert(player_it.game_data.is_round_over == TRUE, "Last round is not over");
    
    _players.modify(player_it, same_payer, [&](auto &content) {
        game_info game_data = content.game_data;
        // game_data.round_id += 1;  // round_id不在此action更新
        if (soldier_type == 0)
        {
          // 玩家未购买士兵，系统默认认为soldier_type为0
          eosio_assert(game_data.soldiers_cellar.begin() == game_data.soldiers_cellar.end(), "Error in picksoldier");
        }
        else
        {
          // 确保仓库中存在该种士兵
          int count = std::count(game_data.soldiers_cellar.begin(), game_data.soldiers_cellar.end(), soldier_type);
          eosio_assert(count != 0, "You don't have this type of soldier!");
          vector<soldier>::iterator it;
          it = std::find(game_data.soldiers_cellar.begin(), game_data.soldiers_cellar.end(), soldier_type);
          game_data.soldiers_cellar.erase(it);         //使用过的兵种不可以再使用，要从仓库中删除
        }
        
        game_data.soldier_selected = soldier_type;
        game_data.is_round_over = FALSE;    // 本轮开启标志
        content.game_data = game_data;
    });
}

void EOSSiege::getresult(name attacker_name, name defender_name, uint64_t city_id, asset attacker_eos, asset defender_eos)
{
    //Authrozied
    // require_auth(player_name);
    require_auth(_self);
    // 确保双方不在冻结名单
    eosio_assert(_frozen.find(attacker_name.value) == _frozen.end(), "The attacker is in the frozen list");
    eosio_assert(_frozen.find(defender_name.value) == _frozen.end(), "The defender is in the frozen list");
    
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.game_stage == RUNNING, "game is not in running stage");
    
    auto &attacker_it = _players.get(attacker_name.value, "Cannot find the attacker!");
    auto &defender_it = _players.get(defender_name.value, "Cannot find the defender!");
    auto &city_it = _cities.get(city_id, "Cannot find the city!");
    
    // 确保双方处于in_battle状态
    eosio_assert(attacker_it.in_battle == TRUE, "The attacker is not in any battle");
    eosio_assert(defender_it.in_battle == TRUE, "The defender is not in any battle");
    
    // 确保双方处于同一round
    eosio_assert(attacker_it.game_data.round_id == defender_it.game_data.round_id, "Not in the same round!");
    eosio_assert(attacker_it.game_data.round_id <= 5, "This battle is over");
    eosio_assert(attacker_it.game_data.is_round_over == FALSE, "This round is over");
    eosio_assert(defender_it.game_data.is_round_over == FALSE, "This round is over");

    //Get the soldier
    soldier attacker_soldier = attacker_it.game_data.soldier_selected;
    soldier defender_soldier = defender_it.game_data.soldier_selected;
    int result = battleresult(attacker_soldier, defender_soldier);
    
    // 获取本轮结果
    if (result == 1)
    {
      // attacker获胜
      _players.modify(attacker_it, same_payer, [&](auto &content){
        game_info game_data = content.game_data;
        game_data.is_round_over = TRUE;    // 本轮结束
        game_data.soldier_selected = 0;    // 重置所选的兵种
        game_data.round_id += 1;           // round加1
        content.game_data = game_data;
      });
        
      _players.modify(defender_it, same_payer, [&](auto &content){
        game_info game_data = content.game_data;
          
        uint64_t soldier_point = soldiers_point.at(defender_soldier);
        game_data.current_soldiers_point -= soldier_point;
        game_data.is_round_over = TRUE;
        game_data.soldier_selected = 0;
        game_data.round_id += 1;
        content.game_data = game_data;
      });
    }
    else if(result == -1)
    {
      // attacker失败
      _players.modify(attacker_it, same_payer, [&](auto &content){
        game_info game_data = content.game_data;
          
        uint64_t soldier_point = soldiers_point.at(attacker_soldier);
        game_data.current_soldiers_point -= soldier_point;
        game_data.is_round_over = TRUE;
        game_data.soldier_selected = 0;
        game_data.round_id += 1;
        content.game_data = game_data;
      });
        
      _players.modify(defender_it, same_payer, [&](auto &content){
        game_info game_data = content.game_data;
        game_data.is_round_over = TRUE;
        game_data.soldier_selected = 0;
        game_data.round_id += 1;
        content.game_data = game_data;
      });
    }
    else if(result == 0)
    {
      // 双方战平
      _players.modify(attacker_it, same_payer, [&](auto &content){
        game_info game_data = content.game_data;
          
        // 设定双方战平均减去战力(此处稍微对进攻者有利)
        uint64_t soldier_point = soldiers_point.at(attacker_soldier);
        game_data.current_soldiers_point -= soldier_point;
        game_data.is_round_over = TRUE;
        game_data.soldier_selected = 0;
        game_data.round_id += 1;
        content.game_data = game_data;
      });
        
      _players.modify(defender_it, same_payer, [&](auto &content){
        game_info game_data = content.game_data;
          
        uint64_t soldier_point = soldiers_point.at(defender_soldier);
        game_data.current_soldiers_point -= soldier_point;
        game_data.is_round_over = TRUE;
        game_data.soldier_selected = 0;
        game_data.round_id += 1;
        content.game_data = game_data;
      });
    }
    else
    {
      // Error
    }
    
    // 判断这场战斗是否结束，标志为任意一方仓库为空
    if((attacker_it.game_data.soldiers_cellar.begin() == attacker_it.game_data.soldiers_cellar.end()) 
    || (defender_it.game_data.soldiers_cellar.begin() == defender_it.game_data.soldiers_cellar.end()))
    {
      // 战斗结束
      point attacker_current_point = attacker_it.game_data.current_soldiers_point * 100;    // 放大倍数
      point defender_current_point = defender_it.game_data.current_soldiers_point * city_defense_index_map.at(city_id);
      
      if(attacker_current_point > defender_current_point)
      {
        // attacker获得这场战斗胜利
        // defender left city
        uint64_t produced_bonus = city_it.produced_bonus;
        auto quantity = asset(produced_bonus, symbol("EOS", 4));
        
        if(quantity.amount != 0)
        {
          action(
            permission_level{_self, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple(_self, defender_name, quantity,
                            std::string("The defender has got the bonus and left the city!")))
            .send();
        }
        
        //reset defender's data
        _players.modify(defender_it, same_payer, [&](auto &content) {
          content.is_attacker = FALSE;
          content.is_defender = FALSE;
          content.opponent = name(0);
          
          content.be_attacked_request = FALSE;
          content.before_battle = FALSE;
          content.in_battle = FALSE;
          content.own_city_id = 0;
            
          game_info game_data;
          content.game_data = game_data;
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
            
            content.in_battle = FALSE;
            content.own_city_id = city_id;
            
            game_info game_data;
            content.game_data = game_data;
        });
        
        // defender购买士兵的eos输给attacker, attacker购买士兵的eos充入奖池
        if(defender_eos.amount != 0)
        {
          action(
            permission_level{"temptreasure"_n, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple("temptreasure"_n, attacker_name, defender_eos,
                            std::string("defender has lost his eos to the attacker!")))
            .send();
        }
        
        if(attacker_eos.amount != 0)
        {
          action(
            permission_level{"temptreasure"_n, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple("temptreasure"_n, _self, attacker_eos,
                            std::string("attacker has transfered his eos to the bonus pool!")))
            .send();
        }
      }
      else if(attacker_current_point < defender_current_point)
      {
        // defender获得这场战斗胜利
        // modify defender's data
        _players.modify(defender_it, same_payer, [&](auto &content){
          content.opponent = name(0);
          
          content.in_battle = FALSE;
          
          game_info game_data;
          content.game_data = game_data;
        });
        
        //reset attacker's data
        _players.modify(attacker_it, same_payer, [&](auto &content){
          content.is_attacker = FALSE;
          content.is_defender = FALSE;
          content.opponent = name(0);
          
          content.be_attacked_request = FALSE;
          content.before_battle = FALSE;
          content.in_battle = FALSE;
          content.own_city_id = 0;
            
          game_info game_data;
          content.game_data = game_data;
        });
        
        // attacker购买士兵的eos输给defender，defender购买士兵的eos充入奖池
        if(attacker_eos.amount != 0)
        {
          action(
            permission_level{"temptreasure"_n, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple("temptreasure"_n, defender_name, attacker_eos,
                            std::string("attacker has lost his eos to the defender!")))
            .send();
        }
        if(defender_eos.amount != 0)
        {
          action(
            permission_level{"temptreasure"_n, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple("temptreasure"_n, _self, defender_eos,
                            std::string("defender has transfered his eos to the bonus pool!")))
            .send();
        }
      }
      else if(attacker_current_point == defender_current_point)
      {
        // 双方战平
        // modify defender's data
        _players.modify(defender_it, same_payer, [&](auto &content){
          content.opponent = name(0);
          
          content.in_battle = FALSE;
          
          game_info game_data;
          content.game_data = game_data;
        });
        
        //reset attacker's data
        _players.modify(attacker_it, same_payer, [&](auto &content){
          content.is_attacker = FALSE;
          content.is_defender = FALSE;
          content.opponent = name(0);
          
          content.be_attacked_request = FALSE;
          content.before_battle = FALSE;
          content.in_battle = FALSE;
          content.own_city_id = 0;
            
          game_info game_data;
          content.game_data = game_data;
        });
        
        // 双方拿回购买士兵的eos
        if(defender_eos.amount != 0)
        {
          action(
            permission_level{"temptreasure"_n, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple("temptreasure"_n, defender_name, defender_eos,
                            std::string("defender has token back his eos!")))
            .send();
        }
        if(attacker_eos.amount != 0)
        {
          action(
            permission_level{"temptreasure"_n, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple("temptreasure"_n, attacker_name, attacker_eos,
                            std::string("attacker has token back his eos!")))
            .send();
        }
      }
      else
      {
        // Error
        // 双方拿回购买士兵的eos
        if(defender_eos.amount != 0)
        {
          action(
            permission_level{"temptreasure"_n, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple("temptreasure"_n, defender_name, defender_eos,
                            std::string("defender has token back his eos!")))
            .send();
        }
        if(attacker_eos.amount != 0)
        {
          action(
            permission_level{"temptreasure"_n, "active"_n},
            "eosio.token"_n, "transfer"_n,
            std::make_tuple("temptreasure"_n, attacker_name, attacker_eos,
                            std::string("attacker has token back his eos!")))
            .send();
        }
      }
    }
}

void EOSSiege::settlement(name player_name, uint64_t city_id)
{
  require_auth(_self);
  // 确保玩家不在冻结名单
  eosio_assert(_frozen.find(player_name.value) == _frozen.end(), "You are in frozen list");
  
  auto &player_it = _players.get(player_name.value, "Cannot find the player!");
  auto &city_it = _cities.get(city_id, "Cannot find the city!");
  auto &global_it = _global.get(1, "Error when check global table!");
  eosio_assert(global_it.game_stage == RUNNING, "game stage is not in running");
  
  eosio_assert(city_it.belong_player == player_name, "You don't have this city");
  uint64_t produced_bonus = city_it.produced_bonus;
  auto quantity = asset(produced_bonus, symbol("EOS", 4));
  
  // 游戏状态切换至SETTLING
  _global.modify(global_it, same_payer, [&](auto &content){
    content.game_stage = SETTLING;
  });
  
  // 确保转账能够成功
  if(quantity.amount != 0)
  {
    action(
      permission_level{_self, "active"_n},
      "eosio.token"_n, "transfer"_n,
      std::make_tuple(_self, player_name, quantity,
                      std::string("game over and you take your bonus")))
      .send();
  }
  
  _cities.modify(city_it, same_payer, [&](auto &content){
    content.if_be_occupied = FALSE;
    content.belong_player = name(0);
    content.produced_bonus = 0;
  });
}

void EOSSiege::allend()
{
    require_auth(_self);
    auto &global_it = _global.get(1, "Error when check global table!");
    eosio_assert(global_it.game_stage == SETTLING, "game stage is not in settling");
    
    /* delete the cities table */
    auto cities_begin_it = _cities.begin();
    while (cities_begin_it != _cities.end())
    {
        cities_begin_it = _cities.erase(cities_begin_it);
    }

    // auto global_begin_it = _global.begin();
    // while (global_begin_it != _global.end())
    // {
    //     global_begin_it = _global.erase(global_begin_it);
    // }

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
    
    // 更新全局表
    _global.modify(global_it, same_payer, [&](auto &content){
      content.cities_remain = CITY_NUM;
      content.game_stage = END;
      content.produce_rate = 0.0;
    });
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
            case "allocatecity"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::allocatecity);
                break;
            case "freezeplayer"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::freezeplayer);
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
            case "settlement"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::settlement);
                break;
            case "allend"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::allend);
                break;
            case "temp"_n.value:
                execute_action(name(receiver), name(code), &EOSSiege::temp);
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




