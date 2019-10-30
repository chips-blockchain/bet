struct cashier *cashier_info;

#if 0
void BET_cashier_loop(void * _ptr)
{
	int32_t recvlen=0; 
	void *ptr=NULL; 
	cJSON *msgjson=NULL; struct cashier* cashier_info= _ptr;
    uint8_t flag=1;

	
    while ( flag )
    {
        
        if ( cashier_info->subsock >= 0 && cashier_info->pushsock >= 0 )
        {
        		ptr=0;
				char *tmp=NULL;
	        	recvlen= nn_recv (bet->subsock, &ptr, NN_MSG, 0);
				if(recvlen>0)
					tmp=clonestr(ptr);
                if ((recvlen>0) && ((msgjson= cJSON_Parse(tmp)) != 0 ))
                {
                    if ( BET_cashier_backend(msgjson,bet,Player_VARS_global) < 0 )
                    {
                    	printf("\nFAILURE\n");
                    	// do something here, possibly this could be because unknown commnad or because of encountering a special case which state machine fails to handle
                    }           
                    if(tmp)
						free(tmp);
					if(ptr)
						nn_freemsg(ptr);
					
                }
                
        }
        
    }
}

#endif
