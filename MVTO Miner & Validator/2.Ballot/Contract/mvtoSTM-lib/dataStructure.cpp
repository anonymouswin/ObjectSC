#include "dataStructure.hpp"

/*
 * Constructor of the class G_node.
 */
G_node::G_node(int key, voidVal* tb)
{
	this->G_key     = key;
	this->red_next  = NULL;
	this->blue_next = NULL;
	this->G_mark    = DEFAULT_MARKED;
	//push the default version back to the G_node.
	version *T0_version  = new version(tb->size);
	T0_version->G_ts     = 0;
	T0_version->G_val    = 0;
	T0_version->G_max_RV = 0;
	copyBytes(T0_version->val, tb->val, tb->size);

	this->G_vl->push_back(T0_version);
}
/*
 * Method to compare two G_nodes on the basis of their keys.
 */
bool G_node::compareG_nodes(G_node* node)
{
	if(this->G_key == node->G_key)
	{
		return true;
	}
	return false;
}

/**********************************************************************
 *********************************************************************/

/*
 * Constructor of the class HashMap.
 */
HashMap::HashMap(voidVal* tb)
{
	htable = new G_node* [TABLE_SIZE];
	//Initialize head and tail sentinals nodes for all the buckets of HashMap.
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		htable[i] = NULL;
		htable[i] = new G_node(INT_MIN, tb);
		htable[i]->red_next  = new G_node(INT_MAX, tb);
		htable[i]->blue_next = htable[i]->red_next;
	}
}

/*
 * Destructor of the class HashMap.
 */
HashMap::~HashMap()
{
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		if (htable[i] != NULL)
		{
			G_node *prev  = NULL;
			G_node *entry = htable[i];
			while (entry != NULL)
			{
				prev  = entry;
				entry = entry->red_next;
				delete prev;
			}
		}
	}
	delete[] htable;
}

/*
 * Function to define the object Id or bucket number.
 */
int HashMap::HashFunc(int key)
{
	return key % TABLE_SIZE;
}

/*
 *Insert Key-Node in the appropriate position with a default version.
 */
void HashMap::list_Ins(int L_key, int *L_val, G_node** G_preds, G_node** G_currs, LIST_TYPE list_type, int L_tx_id, voidVal* tb)
{

	//Inserting the node from redList to blueList
	if(RL_BL == list_type)
	{
		G_currs[0]->blue_next = G_currs[1];
		G_preds[0]->blue_next = G_currs[0];
		G_currs[0]->G_mark    = false;
		assert(L_key == G_currs[0]->G_key);
	}
	//Inserting the node in redlist only.
	else if(RL == list_type)
	{
		G_node *node = new G_node(L_key, tb);
		//After creating node lock it.
		node->lmutex.lock();
		//Add current transaction to the rv-list of the 0th version.
		node->G_vl->at(0)->G_rvl->push_back(L_tx_id);

//		node->G_vl->at(0)->rList->push_back(L_tx_id);
		//!========================================
		node->G_vl->at(0)->G_max_RV = L_tx_id;
		//!========================================

		node->G_mark   = true;
		node->red_next = G_currs[0];
		G_currs[0]->lmutex.unlock();
		G_currs[0] = node;
		G_preds[1]->red_next = node;
	}
	//Inserting the node in red as well as blue list.
	else
	{
		G_node *node = new G_node(L_key, tb);
		//After creating node lock it.
		node->lmutex.lock();
		//Add another version with current transaction's timestamp.
		version *T_key_version = new version(tb->size);
		T_key_version->G_ts    = L_tx_id;

		copyBytes(T_key_version->val, tb->val, tb->size);

//		node->G_vl->at(0)->rList->push_back(L_tx_id);
//		node->G_vl->at(0)->G_rvl->push_back(L_tx_id);
		//!========================================
		T_key_version->G_max_RV = L_tx_id;
		//!========================================

		if(L_val != NULL)
		{
			T_key_version->G_val = *L_val;
			copyBytes(T_key_version->val, tb->val, tb->size);
		}
		else
		{
			T_key_version->G_val = -1;
			copyBytes(T_key_version->val, tb->val, tb->size);
		}
		//Push the current timestamp version to the node's RV list.
		node->G_vl->push_back(T_key_version);
		node->red_next        = G_currs[0];
		node->blue_next       = G_currs[1];
		G_preds[1]->red_next  = node;
		G_preds[0]->blue_next = node;
	}
}

/*
 * Funtion to determine preds and currs.
 */
OPN_STATUS HashMap::list_LookUp(int L_bucket_id, int L_key, G_node** G_preds, G_node** G_currs)
{
	OPN_STATUS op_status = RETRY;
	G_node *head         = NULL;

	//If key to be searched is not in the range between -infinity to +infinity.
	if((L_key <= INT_MIN) && (L_key >= INT_MAX))
	{
		assert((L_key > INT_MIN) && (L_key < INT_MAX));
	}

	//Run until status of operation doesn't change from RETRY.
	while(RETRY == op_status)
	{
		//Get the head of the bucket in chaining hash
		//table with the help of L_obj_id and L_key.
		//if bucket is empty
		if (htable[L_bucket_id] == NULL)
		{
			elog("bucket is empty \n");
			return BUCKET_EMPTY;
		}
		else
		{
			head = htable[L_bucket_id];
		}

		G_preds[0] = head;
		G_currs[1] = G_preds[0]->blue_next;

		//search blue pred and curr
		while(G_currs[1]->G_key < L_key)
		{
			G_preds[0] = G_currs[1];
			G_currs[1] = G_currs[1]->blue_next;
		}
		G_preds[1]     = G_preds[0];
		G_currs[0]     = G_preds[0]->red_next;

		//search red pred and curr
		while(G_currs[0]->G_key < L_key)
		{
			G_preds[1] = G_currs[0];
			G_currs[0] = G_currs[0]->red_next;
		}

		//Acquire locks on all preds and currs.
		acquirePredCurrLocks(G_preds, G_currs);

		//pred curr or method validation
		op_status = methodValidation(G_preds, G_currs);

		//If op_status is still RETRY and not OK
		//release all the laquired locks and try again.
		if(RETRY == op_status)
		{
			releasePredCurrLocks(G_preds, G_currs);
		}
	}

	#if DEBUG_LOGS
	cout<<"\nlist_LookUp:: nodes "<<G_preds[0]->G_key<<" "<< G_preds[1]->G_key
		<<" "<<G_currs[0]->G_key<<" "<<G_currs[1]->G_key<<endl;
	#endif // DEBUG_LOGS
	return op_status;
}

/*
 * Identify the right version of a G_node that is
 * largest but less than current transaction id.
 */
version* HashMap::find_lts(int L_tx_id, G_node *G_curr)
{
	//Initialize the closest tuple.
	version *closest_tuple = new version(G_curr->G_vl->at(0)->size);
	closest_tuple->G_ts    = 0;
	
	//For all the versions of G_curres[] identify 
	//the largest timestamp less than L_tx_id.
	for(int i = 0; i < G_curr->G_vl->size(); i++)
	{
		int p = G_curr->G_vl->at(i)->G_ts;
		if((p < L_tx_id) && (closest_tuple->G_ts <= p))
		{
			closest_tuple->G_ts  = G_curr->G_vl->at(i)->G_ts;
			closest_tuple->G_val = G_curr->G_vl->at(i)->G_val;
			closest_tuple->G_rvl = G_curr->G_vl->at(i)->G_rvl;

			copyBytes(closest_tuple->val, G_curr->G_vl->at(i)->val, G_curr->G_vl->at(i)->size);
			closest_tuple->rList = G_curr->G_vl->at(i)->rList;
			//!=====================================================
			closest_tuple->G_max_RV = G_curr->G_vl->at(i)->G_max_RV;
			//!=====================================================
		}
	}
	return closest_tuple;
}

/*
 * This method does below steps ->
 * 1. Check if key exists or not, if yes check for the LTS version to read from.
 * 2. Else create the key and its respective version to be read from.
 * 3. If version to be read from is found then add current trans to its RV list.
 */

OPN_STATUS HashMap:: commonLuNDel(int L_tx_id, int obj_id, int L_key, int* L_val, G_node** G_preds, G_node** G_currs, voidVal* tb)
{
	//Operation status to be returned.
	OPN_STATUS L_opn_status;

	//If node corresponding to the key is not present in local log
	//then search into underlying DS with the help of listLookUp().
	L_opn_status = list_LookUp(obj_id, L_key, G_preds, G_currs);
	
	//If node corresponding to the key is part of BL.
	if(G_currs[1]->G_key == L_key)
	{
		version *closest_tuple = find_lts(L_tx_id, G_currs[1]);
		closest_tuple->G_rvl->push_back(L_tx_id);

		//!=======================================
		if(closest_tuple->G_max_RV < L_tx_id)
			closest_tuple->G_max_RV = L_tx_id;
		//!=======================================
		copyBytes(tb->val, closest_tuple->val, closest_tuple->size);

		//If the Key-Node mark field is TRUE then L_op_status and L_val set as
		//FAIL and NULL otherwise set OK and value of closest_tuple respectively.
		if(G_currs[1]->G_mark == true)
		{
			L_opn_status = FAIL;
			*L_val = 0;
		} 
		else 
		{
			L_opn_status = OK;
			*L_val = (closest_tuple->G_val);
		}
	}
	//If node corresponding to the key is part of RL.
	else if(G_currs[0]->G_key == L_key)
	{
		version *closest_tuple = find_lts(L_tx_id,G_currs[0]);
		closest_tuple->G_rvl->push_back(L_tx_id);

		//!=======================================
		if(closest_tuple->G_max_RV < L_tx_id)
			closest_tuple->G_max_RV = L_tx_id;
		//!=======================================
		copyBytes(tb->val, closest_tuple->val, closest_tuple->size);

		//If the Key-Node mark field is TRUE then L_op_status and L_val set as
		//FAIL and NULL otherwise set OK and value of closest_tuple respectively.
		if(G_currs[0]->G_mark != true)
		{
			L_opn_status = OK;
			*L_val = (closest_tuple->G_val);
		}
		else
		{
			L_opn_status = FAIL;
			*L_val = 0;
		}
	}
	//If node corresponding to the key is not part of RL as well as
	//BL then create the node into RL with the help of list_Ins().
	else
	{
		//Insert the version tuple for transaction T0.
		list_Ins(L_key, L_val, G_preds, G_currs, RL, L_tx_id, tb);
		L_opn_status = FAIL;
		*L_val = 0;
	}

	//Releasing the locks in the incresing order.
	releasePredCurrLocks(G_preds, G_currs);
	return OK;
}

/*
 *Method to find the closest tuple created by transaction Tj with the largest timestamp smaller than L_ix_id.
 */
bool HashMap::check_version(int L_tx_id, G_node* G_curr)
{
	/*Identify the tuple that has highest timestamp but less than itself.*/
	version *closest_tuple = find_lts(L_tx_id,G_curr);
	if(closest_tuple != NULL)
	{
		for(int i=0; i<closest_tuple->G_rvl->size();i++)
		{
			int Tk_ts = closest_tuple->G_rvl->at(i);
			/*If in the RV list there exists a transaction with higher ts then the current transaction then return false;*/
			if(L_tx_id < Tk_ts)
			{
				return false;
			}
		}
	}
	/*If in the rv list all the transactions are of lower ts then the current transaction then return true.*/
	return true;
}

/*
 * Method to add a version in the appropriate key_node version list in sorted order.
 */
void HashMap::insertVersion(int L_tx_id, int L_val, G_node* key_Node, list<int>&conf_list, voidVal* tb)
{
	version *newVersion  = new version(tb->size);
	newVersion->G_ts     = L_tx_id;
	newVersion->G_val    = L_val;
	//!===============================
	newVersion->G_max_RV = L_tx_id;
	newVersion->rList->push_back(L_tx_id);
	copyBytes(newVersion->val, tb->val, tb->size);
	//!===============================
	int i = 0;
	for(i = 0; i < key_Node->G_vl->size(); i++)
	{
		/*Look for an appropriate place to put the new version in orer to maintain the order of the version list.*/
		if(key_Node->G_vl->at(i)->G_ts > L_tx_id)
		{
			conf_list.push_back(key_Node->G_vl->at(i)->G_ts);
			int j = i-1;//largest time stamp smaller then L_tx_id.
			if( j > -1 )//with in range
			{
				for(auto it = key_Node->G_vl->at(j)->rList->begin(); it != key_Node->G_vl->at(j)->rList->end(); it++)
					conf_list.push_back(*it);
			}
			key_Node->G_vl->insert(key_Node->G_vl->begin()+i, newVersion);
			return;
		}
	}
	/*Else push at the last.*/
	if(i == key_Node->G_vl->size())
	{
		for(auto it = key_Node->G_vl->at(i-1)->rList->begin(); it != key_Node->G_vl->at(i-1)->rList->end(); it++)
			conf_list.push_back(*it);
	}
	key_Node->G_vl->push_back(newVersion);
}

/*
 *Function to delete a node from blue link and place it in red link after marking it.
 */
void HashMap::list_Del(G_node** G_preds, G_node** G_currs)
{
    G_preds[0]->blue_next = G_currs[1]->blue_next;
}

/**
 * Print the current table contents.
 **/
void HashMap::printHashMap(int L_bucket_id)
{
	G_node *head = NULL;
	
	if (htable[L_bucket_id] == NULL)
	{
		elog("bucket is empty \n");
		return;
	}
	else
	{
		head = htable[L_bucket_id];
	}

	while(head->G_key < INT_MAX)
	{
		cout<<"Key "<<head->G_key<<" ";
		for(int i=0;i<head->G_vl->size();i++)
		{
			cout<<head->G_vl->at(i)->G_ts<<" ";
		}
		cout<<endl;
		head = head->red_next;
	}
}

OPN_STATUS HashMap::tidUpdate(int key, int L_tx_id, G_node* G_curr, int L_BucketId)
{
	G_node *head = NULL;
	if (htable[L_BucketId] == NULL)
	{
		elog("bucket is empty \n");
		return FAIL;
	}
	else
	{
		head = htable[L_BucketId];
	}

	while(head->G_key < key)
	{
		
		if(head->G_vl->size() > 0)
		{
			int i = 0;
			for(; i < head->G_vl->size(); i++)
			{
				if(head->G_vl->at(i)->G_ts > L_tx_id)
				{
					break;
				}
			}
			if(i > 1)
			{
				i = i-1;
				head->lmutex.lock();
					head->G_vl->at(i)->G_rvl->push_back(L_tx_id);
					head->G_vl->at(i)->rList->push_back(L_tx_id);
				head->lmutex.unlock();
			}
		}
		head = head->red_next;
	}
}
/***********************************************************************
 **********************************************************************/

/*
 *Function to acquire all locks taken during listLookUp().
 */
void acquirePredCurrLocks(G_node** G_preds, G_node** G_currs)
{
	G_preds[0]->lmutex.lock();
	G_preds[1]->lmutex.lock();
	G_currs[0]->lmutex.lock();
	G_currs[1]->lmutex.lock();
}

/*
 *Function to release all locks taken during listLookUp().
 */
void releasePredCurrLocks(G_node** G_preds, G_node** G_currs)
{
	G_preds[0]->lmutex.unlock();
	G_preds[1]->lmutex.unlock();
	G_currs[0]->lmutex.unlock();
	G_currs[1]->lmutex.unlock();
}

/*
 *This method identifies the conflicts among the concurrent methods of different transactions.
 */
OPN_STATUS methodValidation(G_node** G_preds, G_node** G_currs)
{
	//Validating g_pred[] and G_currs[].
	if((G_preds[0]->G_mark) || (G_currs[1]->G_mark) || (G_preds[0]->blue_next != G_currs[1]) || (G_preds[1]->red_next != G_currs[0]))
	{
		return RETRY;
	}
	else
		return OK;
}
