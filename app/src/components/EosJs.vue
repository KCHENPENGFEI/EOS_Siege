<template>
  <button @click="updateRankTb">测试</button>
</template>

<script>
import EosService from "../service/eos";
export default {
  methods: {
    async updateRankTb() {
      var ranking = 4;
      var player = "player2";
			var bidding_price = "0.8100 EOS";
			var now = new Date();

			function timetrans(time){
				var year = time.getFullYear().toString();
				var month = (time.getMonth() + 1).toString();
				var day = time.getDate().toString();
				var hour = time.getHours().toString();
				var minute = time.getMinutes().toString();
				var second = time.getSeconds().toString();
				return year + '-' + month + '-' + day + ' ' + hour + ':' + minute + ':' + second; 
			}
			var bidding_time = timetrans(now);
      var eos = new EosService("eossiege", "eossiege");
      var result = await eos.updateranktb(
        ranking,
        player,
        bidding_price,
        bidding_time
			);
			console.log(result);
    }
  }
};
</script>
