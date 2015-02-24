#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

static mbox mboxs[MBOX_NUM_MBOXES];
static mbox_message mbox_messages[MBOX_NUM_BUFFERS];

//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words, 
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
    int i;
    dbprintf('p', "MboxModuleInit: Entering MboxModuleInit\n");
    for(i=0; i<MBOX_NUM_MBOXES; i++){
        mboxs[i].inuse = 0;
    }
    for(i=0; i< MBOX_NUM_BUFFERS; i++){
        mbox_messages[i].inuse = 0;
    }
    dbprintf('p', "MboxModuleInit: Leaving MboxModuleInit\n");
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use. 
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
    mbox_t mbox;
    uint32 intrval;

    //grabbing a mailbox should be an atomic operation
    intrval = DisableIntrs();
    for(mbox=0; mbox<MBOX_NUM_MBOXES; mbox++){
        if(mboxs[mbox].inuse == 0){
            mboxs[mbox].inuse = 1;
            break;
        }
    }

    RestoreIntrs(intrval);
    if(mbox == MBOX_NUM_MBOXES) return MBOX_FAIL;

    if(AQueueInit(&mboxs[mbox].messages) != QUEUE_SUCCESS){
        printf("FATAL ERROR: Could not initialize mbox messsage queue\n");
        exitsim();
    }

    if(AQueueInit(&mboxs[mbox].pids) != QUEUE_SUCCESS){
        printf("FATAL ERROR: Could not initialize mbox pid queue\n");
        exitsim();
    }

    return mbox;
}

//-------------------------------------------------------
// 
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle 
// of the mailbox and the inuse flag will not be changed 
// during execution.  This allows us to get the a valid 
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {
    Link *l;

    if(!&mboxs[handle]) return MBOX_FAIL;

    if ((l = AQueueAllocLink ((void *)GetCurrentPid())) == NULL) {
        printf("FATAL ERROR: could not allocate link for pid queue in Mbox Open!\n");
        exitsim();
    }

    if (AQueueInsertLast (&mboxs[handle].pids, l) != QUEUE_SUCCESS) {
        printf("FATAL ERROR: could not insert new link into pid queue in Mbox Open!\n");
        exitsim();
    }

    return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
    int i;
    int length;
    Link *l;

    if(!&mboxs[handle]) return MBOX_FAIL;

    if(!AQueueEmpty(&mboxs[handle].pids)){

        length = AQueueLength(&mboxs[handle].pids); 
        
        l = AQueueFirst(&mboxs[handle].pids);

        for(i=0; i < length; i++){
            if((int)AQueueObject(l) == GetCurrentPid()){
                AQueueRemove(&l);
                break;
            }

            l = AQueueNext(l);
        }

    }

    if(AQueueEmpty(&mboxs[handle].pids)){
        while(!AQueueEmpty(&mboxs[handle].messages)){
            l = AQueueFirst(&mboxs[handle].messages);
            AQueueRemove(&l);
        }

        mboxs[handle].inuse = 0;
    }

    return MBOX_SUCCESS;

}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call 
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the 
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {
    int i;
    int qlen;
    Link *l;
    int exists = 0;
    
    //check if mailbox is real
    if(!&mboxs[handle]) return MBOX_FAIL;

    //check if pid opened this mailbox
    if(!AQueueEmpty(&mboxs[handle].pids ) ){
       
        qlen = AQueueLength(&mboxs[handle].pids); 
        
        l = AQueueFirst(&mboxs[handle].pids);

        for(i=0; i < qlen; i++){
            if((int)AQueueObject(l) == GetCurrentPid()){
                exists = 1; 
                break;
            }

            l = AQueueNext(l);
        }
       
       //actuall checks if pid exists 
        if(exists == 0){
            return MBOX_FAIL;
        }

        //check if message longer than max length
        if(length > MBOX_MAX_MESSAGE_LENGTH) {
            return MBOX_FAIL;
        }
        
        //check for space in mailbox (while loop)
        while(AQueueLength(&mboxs[handle].messages) == MBOX_MAX_BUFFERS_PER_MBOX );

                //lock
        if(! LockAcquire(&mboxs[handle].l) ){
            printf("FATAL ERROR: could not get lock in Mbox send!\n");
            exitsim();
        }

        for(i=0; i<MBOX_NUM_BUFFERS; i++){
            if(mbox_messages[i].inuse == 0){
                mbox_messages[i].inuse = 1;
                break;
            }
        }


        //creating mbox_message structure
        dstrncpy(mbox_messages[i].message, (char *) message, length);
        mbox_messages[i].length = length;

        if ((l = AQueueAllocLink(&mbox_messages[i])) == NULL) {
            printf("FATAL ERROR: could not allocate link for pid queue in Mbox Open!\n");
            exitsim();
        }

        //add message to end of queue
        AQueueInsertLast(&mboxs[handle].messages, l);

        //unlock
        if(! LockRelease(&mboxs[handle].l) ){
            printf("FATAL ERROR: could not release lock in Mbox send!\n");
            exitsim();
        }


    }else{
        return MBOX_FAIL;
    }

    return MBOX_SUCCESS;

}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call 
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".  
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox 
// via MboxOpen.
//   
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) {
    int i;
    int qlen;
    Link *l;
    int exists = 0;
    
    //check if mailbox is real
    if(!&mboxs[handle]) return MBOX_FAIL;

    //check if pid opened this mailbox
    if(!AQueueEmpty(&mboxs[handle].pids ) ){
       
        qlen = AQueueLength(&mboxs[handle].pids); 
        
        l = AQueueFirst(&mboxs[handle].pids);

        for(i=0; i < qlen; i++){
            if((int)AQueueObject(l) == GetCurrentPid()){
                exists = 1; 
                break;
            }

            l = AQueueNext(l);
        }
       
       //actuall checks if pid exists 
        if(exists == 0){
            return MBOX_FAIL;
        }

        //check if message longer than max length
        if(length > MBOX_MAX_MESSAGE_LENGTH) {
            return MBOX_FAIL;
        }
        
        //check for space in mailbox (while loop)
        while(AQueueLength(&mboxs[handle].messages) == MBOX_MAX_BUFFERS_PER_MBOX );

                //lock
        if(! LockAcquire(&mboxs[handle].l) ){
            printf("FATAL ERROR: could not get lock in Mbox send!\n");
            exitsim();
        }

        for(i=0; i<MBOX_NUM_BUFFERS; i++){
            if(mbox_messages[i].inuse == 0){
                mbox_messages[i].inuse = 1;
                break;
            }
        }


        //creating mbox_message structure
        dstrncpy(mbox_messages[i].message, (char *) message, length);
        mbox_messages[i].length = length;

        if ((l = AQueueAllocLink(&mbox_messages[i])) == NULL) {
            printf("FATAL ERROR: could not allocate link for pid queue in Mbox Open!\n");
            exitsim();
        }

        //add message to end of queue
        AQueueInsertLast(&mboxs[handle].messages, l);

        //unlock
        if(! LockRelease(&mboxs[handle].l) ){
            printf("FATAL ERROR: could not release lock in Mbox send!\n");
            exitsim();
        }


    }else{
        return MBOX_FAIL;
    }

    return MBOX_SUCCESS;

   //check if pid opened mailbox
    //check if message longer than maxlength
    //lock
    //read message at front of queue
    //unlock

  return MBOX_FAIL;
}

//--------------------------------------------------------------------------------
// 
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {
    //loop through queue of Mbox
    //call mbox close with handle
   // Will pid flow through to the MboxClose?
    // if so then we're golden

  return MBOX_FAIL;
}
