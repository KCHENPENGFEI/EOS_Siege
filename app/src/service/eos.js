import { Api, JsonRpc, RpcError } from "eosjs";
import { JsSignatureProvider } from "eosjs/dist/eosjs-jssig"; // development only



const privateKey = '5JvWPtteXigxP5j1mPmH7pKQZX37jKrA9asAfgDGDEvz2nLykid';
//const privateKey = '5JEkpBNiuJHRmXie11gef8QtBz4TwV4zpk9ipyb34CNCZxRgFcU';
const privateKeys = [privateKey];

const signatureProvider = new JsSignatureProvider(privateKeys);
const rpc = new JsonRpc('http://127.0.0.1:8888', { fetch });
const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() });
//const api = new Api({ rpc, signatureProvider });

const DAPP_NAME = 'EOSSiege';
const CONTRACT_ACCOUNT = 'eossiege';

export default class EosService {
	constructor(dappName, contractAccount) {
		this.dappName = dappName;
		this.contractAccount = contractAccount;
	}

	async updateranktb(ranking, player, bidding_price, bidding_time) {
		// console.log(api);
		const result = await api.transact({
			actions: [{
				account: CONTRACT_ACCOUNT,
				name: 'updateranktb',
				authorization: [{
					actor: CONTRACT_ACCOUNT,
					permission: 'active'
				}],
				data: {
					ranking: ranking,
					player: player,
					bidding_price: bidding_price,
					bidding_time: bidding_time
				},
			}]
		}, {
				blocksBehind: 3,
				expireSeconds: 30
			}).then(() => {
				console.log('update table success!');
				return true;
			}).catch(error => {
				console.log('update table failed, ' + error);
				if (error instanceof RpcError) {
					console.log(JSON.stringify(error.json, null, 2));
				}
				return false;
			});

		return result;
	}

	async allstart() {
		const result = await api.transact({
			actions: [{
				account: CONTRACT_ACCOUNT,
				name: 'allstart',
				authorization: [{
					actor: CONTRACT_ACCOUNT,
					permission: 'active'
				}],
				data: {

				},
			}]
		}, {
				blocksBehind: 3,
				expireSeconds: 30
			}).then(() => {
				console.log('allstart success!');
				return true;
			}).catch(error => {
				console.log('allstart failed, ' + error);
				return false;
			});

		return result;
	}
}