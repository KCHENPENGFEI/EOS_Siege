#ifndef _EOSSIEGE_HPP
#define _EOSSIEGE_HPP

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/symbol.hpp>

#include<string>
#include<vector>
#include<time.h>
//#include<stdlib.h>

#define FALSE false
#define TRUE true
//#define TOKEN_SYMBOL symbol("CHEN", 3)
#define ENTER_FEE 5000       //入场费
#define START_BIDDING_PRICE 2400      //城池起拍价
#define OCCUPATION_FEE 10000     //占领城池费用
#define CITY_NUM 25           //城池数量

using namespace std;
using namespace eosio;

namespace eosiosystem {
  class system_contract;
}

class [[eosio::contract("eossiege")]] EOSSiege : contract {
    private:

    //game satus游戏状态
    enum game_status: int8_t {
        START = 1,
        BIDDING = 2,
        RUNNING = 3,
        END = 4
    };

    //soldier type 兵种：步兵、矛兵、盾兵、弓箭手、骑兵
    enum soldier_type: uint64_t {
        none = 0,
        infantry = 1,
        spearman = 2,
        shieldman = 3,
        archer = 4,
        cavalry = 5
    };

    typedef uint64_t soldier;
    typedef uint64_t point;

    //为了避免精度出现的问题，都采用整数保存和token相关的数据
    const map<soldier, point> soldiers_point = {
        {0, 0},
        {1, 1000},
        {2, 1500},
        {3, 2000},
        {4, 2500},
        {5, 3000}
    };
    
    
    const map<uint64_t, string> city_name_map = {
      {1, "长安"},
      {2, "燕京"},
      {3, "洛阳"},
      {4, "金陵"},
      {5, "荆州"},
      {6, "汴州"},
      {7, "临安"},
      {8, "徐州"},
      {9, "襄阳"},
      {10, "汉中"},
      {11, "咸阳"},
      {12, "益州"},
      {13, "晋阳"},
      {14, "广陵"},
      {15, "长平"},
      {16, "邯郸"},
      {17, "临淄"},
      {18, "曲阜"},
      {19, "寿春"},
      {20, "平城"},
      {21, "当阳"},
      {22, "江陵"},
      {23, "雁门"},
      {24, "夷陵"},
      {25, "东瀛"},
    };

    const map<uint64_t, uint64_t> city_defense_index_map = {
      {1, 120},
      {2, 118},
      {3, 117},
      {4, 116},
      {5, 115},
      {6, 114},
      {7, 112},
      {8, 110},
      {9, 109},
      {10, 108},
      {11, 107},
      {12, 106},
      {13, 100},
      {14, 100},
      {15, 100},
      {16, 100},
      {17, 100},
      {18, 100},
      {19, 100},
      {20, 100},
      {21, 100},
      {22, 100},
      {23, 100},
      {24, 100},
      {25, 100},
    };

    //game_info for every round and every player
    struct game_info
    {
        uint64_t round_id = 0;
        bool prepare_ok = FALSE;
        point all_soldiers_point = 0;     //总兵力
        point current_soldiers_point = 0;     //当前剩余兵力
        uint64_t soldier_quantity = 0;        //士兵数量
        vector<soldier> soldiers_cellar;      //士兵仓库
        soldier soldier_selected = 0;       //本回合选择的士兵
        bool is_round_over = TRUE;       //本回合是否结束
        int8_t status = RUNNING;
    };
    
    // @abi table players
    struct [[eosio::table]] player_info
    {
        name player;
        bool is_attacker = FALSE;
        bool is_defender = FALSE;
        name opponent = name(0);
        
        // 3种状态进行切换
        bool be_attacked_request = FALSE;
        bool before_battle = FALSE;
        bool in_battle = FALSE;
        
        // bool if_win = FALSE;
        // bool if_lost = FALSE;
        
        uint64_t own_city_id = 0;   // 0 means not own any city
        game_info game_data;

        auto primary_key() const {return player.value;}
    };
    
    // @abi table citiess
    struct [[eosio::table]] city_info
    {
        uint64_t city_id;    //range from 1 - 25
        string city_name;
        uint64_t defense_index;
        asset realtime_price = asset(START_BIDDING_PRICE, symbol("EOS", 4));   //The starting price is 0.2400 EOS
        bool if_be_occupied = FALSE;
        name belong_player = name(0);
        //uint64_t city_remain = 5;   //total quantity of cities is 5
        //double produce_rate = 0;
        uint64_t produced_bonus = 1;

        auto primary_key() const {return city_id;}
    };

    // @abi table global
    struct [[eosio::table]] global_info
    {
        uint64_t key = 1;
        uint64_t cities_remain = 25;  // total quantity of cities is 5
        uint64_t game_stage = 0;
        double produce_rate;

        auto primary_key() const {return key;}
    };
    
    // @abi table bidding ranking
    struct [[eosio::table]] bidding_ranking
    {
      uint64_t ranking;
      name player;
      asset bidding_price;
      string bidding_time;
      
      auto primary_key() const {return player.value;}
      // name get_name()
      // {
      //   return player.value;
      // }
    };
    
    // @abi table frozen
    struct [[eosio::table]] frozen_info
    {
      name player;
      uint64_t frozen_rank;
      string frozen_time;
      
      auto primary_key() const { return player.value; }
    };

    typedef multi_index<"players"_n, player_info> players_table;
    typedef multi_index<"cities"_n, city_info> cities_table;
    typedef multi_index<"global"_n, global_info> global_table;
    //typedef multi_index<"rank"_n, bidding_ranking, indexed_by<"player"_n, const_mem_fun<bidding_ranking, name, &bidding_ranking::get_name>>> ranking_table;
    typedef multi_index<"rank"_n, bidding_ranking> ranking_table;
    typedef multi_index<"frozen"_n, frozen_info> frozen_table;
    
    players_table _players;
    cities_table _cities;
    global_table _global;
    ranking_table _rank;
    frozen_table _frozen;


    int battleresult(soldier soldier1, soldier soldier2);
    void split(const string& s, char c, vector<string>& v);

    public:

    using contract::contract;

    // siege( account_name owner ):contract(owner), _players(owner, owner), _cities(owner, owner), _global(owner, owner)
    // {}
    EOSSiege(name receiver, name code, datastream<const char*> ds):contract(receiver, code, ds),
      _players(receiver, code.value), _cities(receiver, code.value), _global(receiver, code.value),
      _rank(receiver, code.value), _frozen(receiver, code.value){}

    [[eosio::action]] void allstart();
    
    [[eosio::action]] void transfer(name from, name to, asset quantity, string memo);
    
    [[eosio::action]] void allocatecity(name player_name, uint64_t city_id, asset price);
    
    [[eosio::action]] void freezeplayer(name player_name, uint64_t rank, string frozen_time);
    
    [[eosio::action]] void updateranktb(uint64_t ranking, name player_name, asset bidding_price, string bidding_time);
    
    [[eosio::action]] void updatestage(uint64_t stage);
    
    [[eosio::action]] void departure(name player_name);

    [[eosio::action]] void leavecity(name player_name);

    [[eosio::action]] void attack(name player_name, name defender_name);

    [[eosio::action]] void defense(name player_name, name attacker_name, uint64_t city_id, uint64_t choice);
    
    [[eosio::action]] void picksoldier(name player_name, soldier soldier_type);

    [[eosio::action]] void getresult(name attacker_name, name defender_name, uint64_t city_id, asset attacker_eos, asset defender_eos);
    
    [[eosio::action]] void temp();

    // [[eosio::action]] void endgame(name player_name);

    [[eosio::action]] void allend();
};

#endif

