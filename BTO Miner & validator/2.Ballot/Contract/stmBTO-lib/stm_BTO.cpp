#include "stm_BTO.h"

/*!==========================================================
 * transaction object constructor: input as transaction id  !
 * creates & initilize local log/buffer for the transaction !
 * ========================================================*/
trans_log:: trans_log( int tid)
{
	//INIT TRANSACTION TID
	this->tid      = tid;
}

/*!===============================
 * TRANSACTION OBJECT DISTRUCTOR !
 * =============================*/
trans_log::~trans_log()
{
	//DELETE trans_log CALL DES FROM ~OSTM
	for(int i = 0; i < this->ll.size(); i++)
	{
		/*temp COMMENT possible seg fault at POval*/
		//deletes preds n currs -->> verify that original
		//shared memory pointers are not freed.
		//this->ll[i].second->~ll_entry();
	}
	//delete[] this->ll;
	/*TEMP comment*/
	//this->ll.clear();

}


/*!=================================
 * DESCP: CREATES NEW LOCAL ENTRY  !
 * ===============================*/
 //a new entry in local Log ll.
int trans_log:: createLLentry(int key,  LinkedHashNode* tb)
{
	ll_entry* log_entry  = new ll_entry;
	log_entry->key       = DEFAULT_KEY;
	log_entry->value     = DEFAULT_VALUE;
	log_entry->opn       = DEFAULT_OPN_NAME;
	log_entry->obj_id    = DEFAULT_VALUE;
	log_entry->bucketId  = DEFAULT_VALUE;
	log_entry->preds     = new LinkedHashNode*[2];
	log_entry->currs     = new LinkedHashNode*[2];
	log_entry->node      = NULL;
	log_entry->op_status = DEFAULT_OP_STATUS;
	log_entry->size      = tb->size;
	log_entry->val       = operator new(tb->size);
	memset((char*)(log_entry->val), 0, tb->size);
	copyBytes(log_entry->val, tb->val, tb->size);
	this->ll.push_back(std::make_pair(key, log_entry));

	//the returned index is used to index into the log vector
	//to access the log entry, thus we return size-1. 
	//As vector statrt with index 0.
	return (this->ll.size() - 1);
}

/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
void trans_log::setPredsnCurrs(int pos, LinkedHashNode** preds, LinkedHashNode** currs)
{
	this->ll[pos].second->preds[0] = preds[0];
	this->ll[pos].second->preds[1] = preds[1];
	this->ll[pos].second->currs[0] = currs[0];
	this->ll[pos].second->currs[1] = currs[1];
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
void trans_log::setOpnName(int pos, OPN_NAME opn)
{
	this->ll[pos].second->opn = opn;
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
void trans_log::setOpStatus(int pos, STATUS op_status)
{
	this->ll[pos].second->op_status = op_status;
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
void trans_log::setValue(int pos, int value, LinkedHashNode* tb)
{
	this->ll[pos].second->value = value;
	copyBytes(this->ll[pos].second->val, tb->val, tb->size);
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
void trans_log::setKey(int pos, int key)
{
	this->ll[pos].second->key = key;
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
void trans_log::setbucketId(int pos, int bid)
{
	this->ll[pos].second->bucketId = bid;
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
int trans_log::findinLL(int key)
{
	for(int i = 0; i < this->ll.size(); i++)
	{
		if(key == this->ll[i].first)
		{
			return i;
		}
	}
	return BAD_INDEX;
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
OPN_NAME trans_log::getOpn(int ll_pos)
{
	if(this->ll[ll_pos].second)
		return this->ll[ll_pos].second->opn;
	else
		return WRONG_OPN;
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
int trans_log::getValue(int ll_pos)
{
	if(this->ll[ll_pos].second)
		return this->ll[ll_pos].second->value;
	else
	{
		std::cout <<"ll[ll_pos].second is NULL"<<std::endl;
		return BAD_VALUE;
	}
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
int trans_log::getKey(int ll_pos)
{
	if(this->ll[ll_pos].second)
		return this->ll[ll_pos].second->key;
	else
	{
		std::cout <<"ll[ll_pos].second is NULL"<<std::endl;
		return BAD_VALUE;
	}
}


/*!================================
 * DESCP: TRANSACTION LOG UTILITY.!
 * ==============================*/
STATUS trans_log::getOpStatus(int ll_pos)
{
	if(this->ll[ll_pos].second)
		return this->ll[ll_pos].second->op_status;
	else
	{
		std::cout <<"ll[ll_pos].second is  NULL"<<std::endl;
		return WRONG_STATUS;
	}
}

/*!======================
 * Used for vector sort.!
 * ====================*/
bool compare_entry(const std::pair<int, ll_entry*> &e1, const std::pair<int,ll_entry*> &e2)
{
	if(e1.second->bucketId != e2.second->bucketId)
		return (e1.second->bucketId < e2.second->bucketId);
	return (e1.first < e2.first);
}


/*!========================
 * DESCP: LIB CONSTRUCTOR.!
 * ======================*/
BTO::BTO(LinkedHashNode* tb)
{
	//shared memory: will init
	//hashtablle as in main.cpp
	voidVal* tb1 = new voidVal( sizeof(tb->size) );
	tid_counter  = 1;
	hash_table   = new HashMap(tb1);
}


/*!==============================================================
 * DESCP: library destructor sorry ;) I ve abused standards and !
 *        havn't used it to free resources. Don't worry nothing !
 *        will go wrong... All works so well ;P                .!
 * ============================================================*/
BTO::~BTO()
{

}


/*!=============================================
 * DESCP: Exported method to begin transaction.!
 * ===========================================*/
trans_log* BTO::begin()
{
	//init the transaction --> tid and local log
	//it is atomic var
	int tid          = tid_counter++;
	trans_log* txlog = new trans_log(tid);
	return txlog;
}




//!=========================================================================
//!==========================  BTO Lib Begin  ==============================
//!=========================================================================

/*!=================================
 * DESCP: Exported method to Abort.!
 * ===============================*/
STATUS BTO::tryAbort(trans_log* txlog)
{
	//causes abort at po validation
	//if release mem here.
	// std::cout<<"abort called"<<std::endl;
	// aborts_count++;
	//txlog->~trans_log();
}


STATUS  BTO::t_read(trans_log* txlog, int obj_id, int key, int* value, LinkedHashNode* tb)
{
	STATUS op_status = ABORT;
	int tid          = txlog->tid;

	//local log:
	int ll_pos = txlog->findinLL(key);

	//If key foubd in local log.
	if(BAD_INDEX != ll_pos)
	{
		OPN_NAME opn = txlog->getOpn(ll_pos);
		//if earlier opr was write
		if(WRITE == opn)
		{
			*value    = txlog->getValue(ll_pos);
			copyBytes(tb->val, txlog->ll[ll_pos].second->val, tb->size);
			op_status = txlog->getOpStatus(ll_pos);
			//Do not update local log as read oprn,
			//keep it write so effect will come for
			//write in try commit in shared memory.
		}
		else if(READ == opn)//if earlier opr was read
		{
			//Node doesnt exist so value is bad.
			*value    = txlog->getValue(ll_pos);
			copyBytes(tb->val, txlog->ll[ll_pos].second->val, tb->size);
 			op_status = txlog->getOpStatus(ll_pos);

 			//Update local log even if we do not update
 			//local log here it do note make any change.
	 		txlog->setKey(ll_pos, key);
			txlog->setOpStatus(ll_pos, op_status);
			txlog->setOpnName(ll_pos, READ);
		}
	}
	//If not found in local log.
	else
	{
		LinkedHashNode* preds[2];
		LinkedHashNode* currs[2];
		*value    = BAD_VALUE;

		//Search in shared memory and obtain locks.
		op_status = hash_table->lslSch(obj_id, key, value, preds, currs, RV, tid);

		if(ABORT == op_status)
		{
			tryAbort(txlog);
			//NOTE: May Shift the unlocks to LslSch().
			preds[0]->lmutex.unlock();
			preds[1]->lmutex.unlock();
			currs[0]->lmutex.unlock();
			currs[1]->lmutex.unlock();

			return ABORT;
		}
		else
		{
			//If in blue list.
			if(key == currs[1]->key)
			{
				op_status = OK;
				//Update max read time stamp.
				copyBytes(tb->val, currs[1]->val, tb->size);

				if(txlog->tid > currs[1]->max_ts.look_ts)
					currs[1]->max_ts.look_ts = txlog->tid;
			}
			//Not required in smart contract.
			//If in red list.
			else if(key == currs[0]->key)
			{
				copyBytes(tb->val, currs[0]->val, tb->size);
				op_status = FAIL;

				if(txlog->tid > currs[0]->max_ts.look_ts)
					currs[0]->max_ts.look_ts = txlog->tid;
			}
			//This case will not occure for Smart Contract.
			//If not on both.
			else
 			{
				hash_table->lslIns(key, 0, preds, currs, RL, tb);
				op_status = FAIL;
				copyBytes(tb->val, preds[1]->val, tb->size);
				//Setting ts of new created node.
				preds[1]->red_next->max_ts.look_ts = txlog->tid;
			}
		}

		//Create ll_entry and log all.
		//New values for subsequent ops.
		int ll_pos = txlog->createLLentry(key, tb);
		txlog->setPredsnCurrs(ll_pos, preds, currs);
		txlog->setOpnName(ll_pos, READ);
		txlog->setValue(ll_pos, *value, tb);
		txlog->setKey(ll_pos, key);
		txlog->setOpStatus(ll_pos, op_status);
		txlog->setbucketId(ll_pos, hash_table->HashFunc(key));

		preds[0]->lmutex.unlock();
		preds[1]->lmutex.unlock();
		currs[0]->lmutex.unlock();
		currs[1]->lmutex.unlock();
	}
	return op_status;
}





STATUS  BTO::t_write(trans_log* txlog, int obj_id, int key, int value, LinkedHashNode* tb)
{
	//local log:
	int ll_pos = txlog->findinLL(key);
	if(BAD_INDEX != ll_pos)
	{
		//Update local log
		//When op_status = OK;
		txlog->setValue(ll_pos, value, tb);
		txlog->setKey(ll_pos, key);
		txlog->setOpStatus(ll_pos, OK);
		txlog->setOpnName(ll_pos, WRITE);
	}
	//The log entry for the key not found.
	//Create a new log entry.
	else
	{
		//Create ll_entry and log all
		//new values for subsequent ops.
		int ll_pos = txlog->createLLentry(key, tb);
		txlog->setOpnName(ll_pos, WRITE);
		txlog->setValue(ll_pos, value, tb);
		txlog->setKey(ll_pos, key);
		txlog->setOpStatus(ll_pos, OK);
		txlog->setbucketId(ll_pos, hash_table->HashFunc(key));
	}
	return OK;
}


STATUS  BTO::bto_TryComit(trans_log* txlog, list<int> &conf_list, LinkedHashNode* tb)
{
	STATUS tx_status = ABORT;
	STATUS op_status = DEFAULT_OP_STATUS;
	int tid          = txlog->tid;

	//Use sorted list now on, as irrelevant
	//the order of opn due to locks.
	sort(txlog->ll.begin(), txlog->ll.end(), compare_entry);

	LinkedHashNode* preds[2];
	LinkedHashNode* currs[2];

	//For all dataitem in txlog do this.
	for(int i = 0; i < txlog->ll.size(); i++)
	{
		//Get key and obj id.
		int key    = txlog->ll[i].first;
		int obj_id = txlog->ll[i].second->obj_id;

		//If opn is READ simply go to next dataitem.
		if((txlog->ll[i].second->opn == READ))
			continue;
		
		//tryComit Validation for BTO is done in lslSch()->Validation().
		//in paper lslSch() => rbl_Search().
		op_status = hash_table->lslSch(obj_id, key, NULL, preds, currs, TRYCOMMIT, tid);

		txlog->setPredsnCurrs(i, preds, currs);

		//op_status is ABORT if Time Stamp validation
		//is failed during lslSch()/rbl_Search().
		if(ABORT == op_status)
		{
			//Release locks and memory if validation fails.
			//Delete all dynamic allocation of transaction.
			tryAbort(txlog);

			//Release lock for all the nodes in the txlog
			//for which locks are obtained till 'i'.
			for(int j = 0; j <= i; j++)
			{
				//If opn is READ simply go to next dataitem.
				if(txlog->ll[j].second->opn == READ)
					continue;
				txlog->ll[j].second->preds[0]->lock_count--;
				txlog->ll[j].second->preds[1]->lock_count--;
				txlog->ll[j].second->currs[0]->lock_count--;
				txlog->ll[j].second->currs[1]->lock_count--;

				txlog->ll[j].second->preds[0]->lmutex.unlock();
				txlog->ll[j].second->preds[1]->lmutex.unlock();
				txlog->ll[j].second->currs[0]->lmutex.unlock();
				txlog->ll[j].second->currs[1]->lmutex.unlock();
			}
			return ABORT;
		}
	}
	//if control reaches here, it means TS validation
	//for all items in write set is successful.
	//NOTE: validation for READ->READ is done in t_read().


	/*!==========================
	 * change the underlying DS !
	 * ========================*/
	//This for loop first do intraTransactionValidation
	//and then based on opn update the DS (shared memory).
	for(int i = 0; i < txlog->ll.size(); i++)
	{
		//get key and obj id
		int key      = txlog->ll[i].first;
		int obj_id   = txlog->ll[i].second->obj_id;
		OPN_NAME opn = txlog->ll[i].second->opn;

		//opn was READ.
		if((txlog->ll[i].second->opn == READ))
			continue;

		/*for multiple update operations within a transaction where
		 *the preds and currs overlap the lost update may happen.*/
		/*previous op was a delete || prev op was WRITE*/
		if((txlog->ll[i].second->preds[0]->marked) || (txlog->ll[i].second->preds[0]->blue_next != txlog->ll[i].second->currs[1]))
		{
			//since locks are recursive before changing the preds0 or
			//currs unlock the pred0 as its sure that the old pred0
			//has been locked multiple times thus, while relasingall
			//locks their would be a correct tracks of number of locks
			//taken and would exactly released same number of time
			//as number of times it was acquired.

			assert( (txlog->ll[i].second->preds[0]->marked) || (txlog->ll[i].second->preds[0]->blue_next != txlog->ll[i].second->currs[1]));
			//assert(txlog->ll[i].second->preds[0]->marked);
			//search previous log entry which is update operation
			//and belongs to same bucket as ith operation.
			int k = -1;
			int j = i-1;
			while(j >= 0)
			{
				//Calculate bucket of the current key and jth key.
				int bi = hash_table->HashFunc(key);
				int bj = hash_table->HashFunc(txlog->ll[j].first);
				//If both belong to same list and th ejth op is not READ.
				if( (bi == bj) && (READ != txlog->ll[j].second->opn))
				{
					k = j;
					break;
				}
				j--;
			}

			//k can never be -1 bcse if this is executed
			//means their is a prev consecutive operation.
			assert(k != -1);

			//if previous op was WRITE
			if(WRITE == txlog->ll[k].second->opn)
			{
				txlog->ll[i].second->preds[0]->lock_count--;

				//ulock the ols pred0 to balance the
				//number of times acquired and release
				txlog->ll[i].second->preds[0]->lmutex.unlock();

				//check to verify correct pred is assigned
				assert(txlog->ll[k].second->preds[0]->blue_next != NULL);
				assert(txlog->ll[k].second->preds[0]->blue_next->key == txlog->ll[k].second->key);

				//again take lock on node*** node field is just used to 
				//maintain lock balance not to set preds[0] as it might
				//be null in case prev node was RL to BL WRITE.
				txlog->ll[i].second->preds[0] = txlog->ll[k].second->preds[0]->blue_next;
				txlog->ll[i].second->preds[0]->lmutex.lock();
				txlog->ll[i].second->preds[0]->lock_count++;
			}
			else
			{
				//if previous op was a DELETE
				//release lock on current pred 0,
				//since pred0 is gonna be changed
				//this node will remain locked forever.
				txlog->ll[i].second->preds[0]->lock_count--;

				//ulock the ols pred0 to balance the
				//number of times acquired and release
				txlog->ll[i].second->preds[0]->lmutex.unlock();

				txlog->ll[i].second->preds[0] = txlog->ll[k].second->preds[0];

				txlog->ll[i].second->preds[0]->lmutex.lock();
				txlog->ll[i].second->preds[0]->lock_count++;

			}

			//if red links have changed find that too
			if(txlog->ll[i].second->preds[1]->red_next != txlog->ll[i].second->currs[0])
			{
				txlog->ll[i].second->preds[1]->lock_count--;

				//unlock the ols pred0 to balance the
				//number of times acquired and release
				txlog->ll[i].second->preds[1]->lmutex.unlock();

				assert(txlog->ll[k].second->preds[1]->red_next != NULL);
				assert(txlog->ll[k].second->preds[1]->red_next->key == txlog->ll[k].second->key );

				txlog->ll[i].second->preds[1] = txlog->ll[k].second->preds[1]->red_next;

				txlog->ll[i].second->preds[1]->lmutex.lock();
				txlog->ll[i].second->preds[1]->lock_count++;
			}

		}

		if(WRITE == opn)
		{
			//fetch value from local log
			//NOTE: test if value is bad value or
			//use get value--txlog->getValue(i);
			int value = txlog->getValue(i);

//			hash_table->tidUpdate(tid, obj_id, key);
			//key already present as blue node
			if(key == txlog->ll[i].second->currs[1]->key)
			{
				txlog->ll[i].second->currs[1]->max_ts.ins_ts = txlog->tid;
				//set value to underlying table node
				txlog->ll[i].second->currs[1]->value = value;
				
				//!=================================================================================
				//::::TODO
				copyBytes(txlog->ll[i].second->currs[1]->val, txlog->ll[i].second->val, txlog->ll[i].second->size);
				//!=================================================================================

				auto it = txlog->ll[i].second->currs[1]->tc_list->begin();
				for(; it != txlog->ll[i].second->currs[1]->tc_list->end(); it++)
					conf_list.push_back(*it);
				
				it = txlog->ll[i].second->currs[1]->rv_list->begin();
				for(; it != txlog->ll[i].second->currs[1]->rv_list->end(); it++)
					conf_list.push_back(*it);
				//!======================================
				//#update this transaction id in TC list.
				//!======================================
				txlog->ll[i].second->currs[1]->tc_list->push_back(tid);

				txlog->setOpStatus(i, OK);
			}
			//key is already present as red node
			else if(key == txlog->ll[i].second->currs[0]->key)
			{
				//int value = txlog->getValue(i);
				hash_table->lslIns(key, value, txlog->ll[i].second->preds, txlog->ll[i].second->currs, RL_BL, tb);

				txlog->ll[i].second->currs[0]->max_ts.ins_ts = txlog->tid;
				
				txlog->ll[i].second->currs[0]->value = value;
				
				//!=================================================================================
				//::::TODO
				copyBytes(txlog->ll[i].second->currs[0]->val, txlog->ll[i].second->val, txlog->ll[i].second->size);
				//!=================================================================================

				auto it = txlog->ll[i].second->currs[0]->tc_list->begin();
				for(; it != txlog->ll[i].second->currs[0]->tc_list->end(); it++)
					conf_list.push_back(*it);
					
				it = txlog->ll[i].second->currs[0]->rv_list->begin();
				for(; it != txlog->ll[i].second->currs[0]->rv_list->end(); it++)
					conf_list.push_back(*it);
				//!======================================
				//#update this transaction id in TC list.
				//!======================================
				txlog->ll[i].second->currs[0]->tc_list->push_back(tid);
				
				txlog->setOpStatus(i, OK);
			}
			//new node for sure added in this
			//case so update ts of the node
			else
			{
				//int value = txlog->getValue(i);
				hash_table->lslIns(key, value, txlog->ll[i].second->preds, txlog->ll[i].second->currs, BL, tb);

				//check the new node is logged indeed correctly and is in DS
				assert(key == txlog->ll[i].second->preds[0]->blue_next->key);

				txlog->ll[i].second->preds[0]->blue_next->max_ts.ins_ts = txlog->tid;

				txlog->ll[i].second->preds[0]->value = value;

				//!======================================
				//#update this transaction id in TC list.
				//!======================================
				txlog->ll[i].second->preds[0]->blue_next->tc_list->push_back(tid);

				//update the node field for WRITEed node in log entry
				txlog->ll[i].second->node = txlog->ll[i].second->preds[0]->blue_next;

				//txlog->ll[i].second->node->lmutex.lock();
				txlog->ll[i].second->node->lock_count++;
				txlog->setOpStatus(i, OK);
			}
		}
	}

	//!===============================================================
	//release all the obtained lock in order. for tryCommit Phase.
	//Why this? because there may be new node added because of WRITE
	//opr and stored as "txlog->ll[i].second->node" in local log.
	//Prepare a vectore with all the lock node and sort, then release.
	//!===============================================================

	//prepare a vector of all preds[], currs[]and node as a pair 
	//<key, hashNode> wherer key is key in hash node. Hashnode is
	//pred, curr or node in txlog.
	std::vector < std::pair< int, LinkedHashNode*> > listOfAllNodes;
	for(int i = 0; i < txlog->ll.size(); i++)
	{
		int k;
		LinkedHashNode* nd;

		//(txlog->ll[i].second->opn == READ)
		if(txlog->ll[i].second->opn == READ)
			continue;

		k  = txlog->ll[i].second->preds[0]->key;
		nd = txlog->ll[i].second->preds[0];
		listOfAllNodes.push_back(std::make_pair(k, nd));

		k  = txlog->ll[i].second->preds[1]->key;
		nd = txlog->ll[i].second->preds[1];
		listOfAllNodes.push_back(std::make_pair(k, nd));

		k  = txlog->ll[i].second->currs[0]->key;
		nd = txlog->ll[i].second->currs[0];
		listOfAllNodes.push_back(std::make_pair(k, nd));

		k  = txlog->ll[i].second->currs[1]->key;
		nd = txlog->ll[i].second->currs[1];
		listOfAllNodes.push_back(std::make_pair(k, nd));

		if(txlog->ll[i].second->node)
		{
			k  = txlog->ll[i].second->node->key;
			nd = txlog->ll[i].second->node;
			listOfAllNodes.push_back(std::make_pair(k, nd));
		}
	}

	//NOTE: sort the all node log by key object as well
	sort(listOfAllNodes.begin(), listOfAllNodes.end());

	//release locks
	for(int i = 0; i < listOfAllNodes.size(); i++)
	{
		listOfAllNodes[i].second->lock_count--;
		listOfAllNodes[i].second->lmutex.unlock();
	}

	//check lock balance for this tx
	#if 0
		std::stringstream msg;
		msg<<"AFTER RELEASE: "<<std::endl;
		std::cout<<msg.str();

		for(int i = 0; i < listOfAllNodes.size(); i++)
		{
			// if(listOfAllNodes[i].second->lock_count)
			{
				std::stringstream msg;
				msg<<"*************Lock Count for key"
				   << listOfAllNodes[i].second->key<<":"
				   <<listOfAllNodes[i].second->lock_count<<std::endl;
				std::cout<<msg.str();
			}
		}

		std::stringstream msg;
		msg<<"CORRECTING"<<std::endl;
		std::cout<<msg.str();

		for(int i = 0; i < listOfAllNodes.size(); i++)
		{
			if(listOfAllNodes[i].second->lock_count)
			{
				while(listOfAllNodes[i].second->lock_count>0)
				{
					listOfAllNodes[i].second->lock_count--;
					listOfAllNodes[i].second->lmutex.unlock();
				}
				std::stringstream msg;
				msg<<"*************Lock Count for key"
				   << listOfAllNodes[i].second->key<<":"
				   <<listOfAllNodes[i].second->lock_count<<std::endl;
				std::cout<<msg.str();
			}
		}
	#endif

	tx_status        = COMMIT;
	txlog->tx_status = COMMIT;

	//#===============================================
	//#RV (read phase-> tid update in RV list
	//#first we need to get the lock on dataitem then
	//#update and finaly release the lock.
	//#===============================================
	for(int i = 0; i < txlog->ll.size(); i++)
	{
		//get key and obj id
		int key      = txlog->ll[i].first;
		int obj_id   = txlog->ll[i].second->obj_id;
		OPN_NAME opn = txlog->ll[i].second->opn;

		//!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//!update this->tid in all the data-itmes rv->list it read/write to "key k".
		//!(data-itmes are shared objects in list DS which are before "key")
		//!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		hash_table->tidUpdate(tid, obj_id, key);
			
		//opn was READ and failed so need to 
		//get lock on red address curr[0].
		if((txlog->ll[i].second->opn == READ)&&(FAIL == txlog->getOpStatus(i)))
		{
			txlog->ll[i].second->currs[0]->lmutex.lock(); //get lock

//			hash_table->tidUpdate(tid, obj_id, key);

			auto it = txlog->ll[i].second->currs[0]->tc_list->begin();
			for(; it != txlog->ll[i].second->currs[0]->tc_list->end(); it++)
				conf_list.push_back(*it);
			txlog->ll[i].second->currs[0]->rv_list->push_back(tid); //update

			txlog->ll[i].second->currs[0]->lmutex.unlock(); //get lock
		}
		//opn was READ and failed so need to 
		//get lock on blue address curr[1].
		else if((txlog->ll[i].second->opn == READ)&&(OK == txlog->getOpStatus(i)))
		{
			txlog->ll[i].second->currs[1]->lmutex.lock(); //get lock

			auto it = txlog->ll[i].second->currs[1]->tc_list->begin();
			for(; it != txlog->ll[i].second->currs[1]->tc_list->end(); it++)
				conf_list.push_back(*it);
			txlog->ll[i].second->currs[1]->rv_list->push_back(tid); //update

			txlog->ll[i].second->currs[1]->lmutex.unlock(); //get lock
		}
	}
	conf_list.sort ();
	conf_list.unique();
	string str = "\nTrns ID = "+to_string(txlog->tid)+" Conflict list {";
	auto it = conf_list.begin();
	for(; it!=conf_list.end(); it++)
		str = str + to_string(*it)+" ";
//	cout<<str<<" }\n";
}





