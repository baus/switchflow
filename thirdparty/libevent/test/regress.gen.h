/*
 * Automatically generated from regress.rpc
 */

#ifndef _REGRESS_RPC_
#define _REGRESS_RPC_

#define EVTAG_HAS(msg, member) ((msg)->member##_set == 1)
#define EVTAG_ASSIGN(msg, member, args...) (*(msg)->member##_assign)(msg, ## args)
#define EVTAG_GET(msg, member, args...) (*(msg)->member##_get)(msg, ## args)
#define EVTAG_ADD(msg, member) (*(msg)->member##_add)(msg)
#define EVTAG_LEN(msg, member) ((msg)->member##_length)

struct msg;
struct kill;
struct run;

/* Tag definition for msg */
enum msg_ {
  MSG_FROM_NAME=1,
  MSG_TO_NAME=2,
  MSG_KILL=3,
  MSG_RUN=4,
  MSG_MAX_TAGS
};

/* Structure declaration for msg */
struct msg {
  char *from_name_data;
  int (*from_name_assign)(struct msg *, const char *);
  int (*from_name_get)(struct msg *, char * *);
  char *to_name_data;
  int (*to_name_assign)(struct msg *, const char *);
  int (*to_name_get)(struct msg *, char * *);
  struct kill *kill_data;
  int (*kill_assign)(struct msg *, const struct kill *);
  int (*kill_get)(struct msg *, struct kill **);
  struct run **run_data;
  int run_length;
  int (*run_assign)(struct msg *, int, const struct run *);
  int (*run_get)(struct msg *, int, struct run **);
  struct run *(*run_add)(struct msg *);

  u_int8_t from_name_set;
  u_int8_t to_name_set;
  u_int8_t kill_set;
  u_int8_t run_set;
};

struct msg *msg_new();
void msg_free(struct msg *);
void msg_clear(struct msg *);
void msg_marshal(struct evbuffer *, const struct msg *);
int msg_unmarshal(struct msg *, struct evbuffer *);
int msg_complete(struct msg *);
void evtag_marshal_msg(struct evbuffer *, u_int8_t, const struct msg *);
int evtag_unmarshal_msg(struct evbuffer *, u_int8_t, struct msg *);
int msg_from_name_assign(struct msg *, const char *);
int msg_from_name_get(struct msg *, char * *);
int msg_to_name_assign(struct msg *, const char *);
int msg_to_name_get(struct msg *, char * *);
int msg_kill_assign(struct msg *, const struct kill *);
int msg_kill_get(struct msg *, struct kill **);
int msg_run_assign(struct msg *, int, const struct run *);
int msg_run_get(struct msg *, int, struct run **);
struct run *msg_run_add(struct msg *);
/* --- msg done --- */

/* Tag definition for kill */
enum kill_ {
  KILL_WEAPON=1,
  KILL_ACTION=2,
  KILL_MAX_TAGS
};

/* Structure declaration for kill */
struct kill {
  char *weapon_data;
  int (*weapon_assign)(struct kill *, const char *);
  int (*weapon_get)(struct kill *, char * *);
  char *action_data;
  int (*action_assign)(struct kill *, const char *);
  int (*action_get)(struct kill *, char * *);

  u_int8_t weapon_set;
  u_int8_t action_set;
};

struct kill *kill_new();
void kill_free(struct kill *);
void kill_clear(struct kill *);
void kill_marshal(struct evbuffer *, const struct kill *);
int kill_unmarshal(struct kill *, struct evbuffer *);
int kill_complete(struct kill *);
void evtag_marshal_kill(struct evbuffer *, u_int8_t, const struct kill *);
int evtag_unmarshal_kill(struct evbuffer *, u_int8_t, struct kill *);
int kill_weapon_assign(struct kill *, const char *);
int kill_weapon_get(struct kill *, char * *);
int kill_action_assign(struct kill *, const char *);
int kill_action_get(struct kill *, char * *);
/* --- kill done --- */

/* Tag definition for run */
enum run_ {
  RUN_HOW=1,
  RUN_MAX_TAGS
};

/* Structure declaration for run */
struct run {
  char *how_data;
  int (*how_assign)(struct run *, const char *);
  int (*how_get)(struct run *, char * *);

  u_int8_t how_set;
};

struct run *run_new();
void run_free(struct run *);
void run_clear(struct run *);
void run_marshal(struct evbuffer *, const struct run *);
int run_unmarshal(struct run *, struct evbuffer *);
int run_complete(struct run *);
void evtag_marshal_run(struct evbuffer *, u_int8_t, const struct run *);
int evtag_unmarshal_run(struct evbuffer *, u_int8_t, struct run *);
int run_how_assign(struct run *, const char *);
int run_how_get(struct run *, char * *);
/* --- run done --- */

#endif  /* _REGRESS_RPC_ */
