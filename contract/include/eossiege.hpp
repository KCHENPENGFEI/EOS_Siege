#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/symbol.hpp>

#include<string>
#include<vector>
//#include<stdlib.h>

#define FALSE false
#define TRUE true
//#define TOKEN_SYMBOL symbol("CHEN", 3)
#define ENTER_FEE 20000       //入场费
#define OCCUPATION_FEE 10000     //占领城池费用
#define CITY_NUM 5           //城池数量

using namespace std;
using namespace eosio;

namespace eosiosystem {
  class system_contract;
}

class [[eosio::contract("eossiege")]] EOSSiege : contract {
    private:

    //game satus游戏状态
    enum game_status: int8_t {
        RUNNING = 1,
        END = 0
    };

    //soldier type 兵种：步兵、矛兵、盾兵、弓箭手、骑兵
    enum soldier_type: uint64_t {
        infantry = 1,
        spearman = 2,
        shieldman = 3,
        archer = 4,
        cavalry = 5
    };

    typedef uint64_t soldier;
    typedef double point;

    const map<soldier, point> soldiers_point = {
        {1, 0.4},
        {2, 0.8},
        {3, 0.8},
        {4, 1.0},
        {5, 1.2}
    };

    //game_info for every round and every player
    struct game_info
    {
        uint64_t round_id = 0;
        bool prepare_ok = FALSE;
        point all_soldiers_point = 0.0;     //总兵力
        point current_soldiers_point = 0.0;     //当前剩余兵力
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
        bool be_attacked_request = FALSE;
        bool in_battle = FALSE;
        name opponent = name(0);
        bool if_win = FALSE;
        bool if_lost = FALSE;
        uint64_t own_city_id = 0;   // 0 means not own any city
        game_info game_data;

        auto primary_key() const {return player.value;}
    };
    
    // @abi table cities
    struct [[eosio::table]] city_info
    {
        uint64_t city_id;    //range from 1 - 5
        bool if_be_occupied = FALSE;
        name belong_player = name(0);
        //uint64_t city_remain = 5;   //total quantity of cities is 5
        //double produce_rate = 0;
        double produced_bonus = 0.01;

        auto primary_key() const {return city_id;}
    };

    // @abi table global
    struct [[eosio::table]] global_info
    {
        uint64_t key = 1;
        uint64_t cities_remain = 5;  // total quantity of cities is 5
        double produce_rate;

        auto primary_key() const {return key;}
    };

    typedef multi_index<"players"_n, player_info> players_table;
    typedef multi_index<"cities"_n, city_info> cities_table;
    typedef multi_index<"global"_n, global_info> global_table;
    
    players_table _players;
    cities_table _cities;
    global_table _global;


    int battleresult(soldier soldier1, soldier soldier2);
    void split(const string& s, char c, vector<string>& v);

    public:

    using contract::contract;

    // siege( account_name owner ):contract(owner), _players(owner, owner), _cities(owner, owner), _global(owner, owner)
    // {}
    EOSSiege(name receiver, name code, datastream<const char*> ds):contract(receiver, code, ds),
      _players(receiver, code.value), _cities(receiver, code.value), _global(receiver, code.value){}

    [[eosio::action]] void allstart();
    
    [[eosio::action]] void transfer(name from, name to, asset quantity, string memo);
    [[eosio::action]] void departure(name player_name);

    //void login(account_name player_name);

    //[[eosio::contract]] void startgame(name player_name);

    //[[eosio::contract]] void occupation(name player_name, uint64_t city_idx);

    

    [[eosio::action]] void leavecity(name player_name);

    //[[eosio::contract]] void buysoldiers(name player_name, soldier soldier_type);

    [[eosio::action]] void attack(name player_name, name defender_name);

    [[eosio::action]] void defense(name player_name, name attacker_name, uint64_t choice);
    
    [[eosio::action]] void picksoldier(name player_name, soldier soldier_type);

    [[eosio::action]] void getresult(name player_name);

    //void nextRound(account_name player_name);

    [[eosio::action]] void endgame(name player_name);

    [[eosio::action]] void allend();
};