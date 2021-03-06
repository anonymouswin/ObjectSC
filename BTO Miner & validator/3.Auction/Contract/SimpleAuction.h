#pragma once
#include <chrono>
#include "stmBTO-lib/stm_BTO.cpp"
using namespace std;


class SimpleAuction
{
	public:
		std::atomic<int> maxBiderID;

		//! using STM-BTO protocol of STM library.
		voidVal* structTB  = new voidVal( sizeof(int) );
		LinkedHashNode* tb = new LinkedHashNode(0, 0, structTB);
		BTO* lib           = new BTO(tb);

		// Parameters of the auction. Times are either absolute unix time-
		// -stamps (seconds since 1970-01-01) or time periods in seconds.		
		std::chrono::time_point<std::chrono::system_clock> now, start;
		
		// beneficiary of the auction.
		int beneficiary;
		int beneficiaryAmount = 0;
		
		
		std::atomic<int> auctionEnd;

		// Current state of the auction (USED BY VALIDATOR).
		std::atomic<int> highestBidder;
		std::atomic<int> highestBid;

		struct PendReturn
		{
			int ID;
			int value;
		};
		// Allowed withdrawals of previous bids.
		// mapping(address => uint) pendingReturns;
		list<PendReturn>pendingReturns;

		// Set to true at the end, disallows any change (USED BY VALIDATOR).
		std::atomic<bool> ended ;

		// The following is a so-called natspec comment,recognizable by the 3
		// slashes. It will be shown when the user is asked to confirm a trans.
		/// Create a simple auction with \`_biddingTime`\, seconds bidding
		/// time on behalf of the beneficiary address \`_beneficiary`\.
		
		// CONSTRUCTOR.
		SimpleAuction( int _biddingTime, int _beneficiary, int numBidder)
		{
			maxBiderID = numBidder;

			//! USED BY VALIDATOR ONLY.
			beneficiary    = _beneficiary;
			highestBidder  = 0;
			highestBid     = 0;
			ended          = false;


			list<int> conf_list;
			trans_log* txlog;
			STATUS ops, txs;
			txlog = lib->begin();

			(*(int*)tb->val) = 0;
			//! \'end'\ in STM memory, USED BY MINERS.
			lib->t_write(txlog, 0, maxBiderID+1, 0, tb);//'end' Sobj

			//! \'highestBidder'\ in STM memory, USED BY MINERS.
			lib->t_write(txlog, 0, maxBiderID+2, 0, tb);//highestBidder

			//! \'highestBid'\ in STM memory, USED BY MINERS.
			lib->t_write(txlog, 0, maxBiderID+3, 0, tb);//highestBid


			for(int b = 1; b <= numBidder; b++)
			{
				//! USED BY VALIDATORS.
				PendReturn pret;
				pret.ID    = b;
				pret.value = 0;
				pendingReturns.push_back(pret);
				
				//! initilize bidder pendingReturns[] value = 0 in STM Memory;				
				//! \'pendingReturns[]'\ memory in STM memory, USED BY MINERS.
				lib->t_write(txlog, 0, b, 0, tb);//highestBid				
			}

			txs = lib->bto_TryComit(txlog, conf_list, tb);

			if(ABORT == txs)
				cout<<"\nError!!Failed to create Shared Object\n";

			start = std::chrono::system_clock::now();
//			cout<<"AUCTION [Start Time = "<<0;
			auctionEnd = _biddingTime;
//			cout<<"] [End Time = "<<auctionEnd<<"] milliseconds";
		};



		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		/*!   FUNCTION FOR VALIDATOR   !*/
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
		// Bid on the auction with the value sent together with transaction.
		// The value will only be refunded if the auction is not won.
		bool bid( int payable, int bidderID, int bidValue );

		// Withdraw a bid that was overbid.
		bool withdraw(int bidderID);
		
		// End the auction and send the highest bid to the beneficiary.
		bool auction_end();
		void AuctionEnded( );
		int send(int bidderID, int amount);
		void reset();

		/*~~~~~~~~~~~~~~~~~~~~~~*/
		/*! FUNCTION FOR MINER.!*/
		/*~~~~~~~~~~~~~~~~~~~~~~*/
		int bid_m( int payable, int bidderID, int bidValue, 
					int *ts, list<int> &cList);
		int withdraw_m(int bidderID, int *ts, list<int> &cList);
		int auction_end_m(int *ts, list<int> &cList);
		bool AuctionEnded_m( );
		int send_m(int bidderID, int amount);

	~SimpleAuction()
	{
		delete lib;
		lib = NULL;
	};//destructor
};
