#pragma once

#ifndef _ADMIN_MODULE_HPP
#define _ADMIN_MODULE_HPP
class admin_module {
private:
  //game satus游戏状态
  enum game_status: int8_t {
    START = 1,
    BIDDING = 2,
    RUNNING = 3,
    SETTLING = 4,
    END = 5
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
  
public:


}

#endif