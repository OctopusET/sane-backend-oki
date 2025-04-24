/*
 * odc_misc.h
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#ifndef ODC_MISC_H_
#define ODC_MISC_H_
#include <stdint.h>
#include <errno.h>
#include <pthread.h>

#include "odc_define.h"
#include "odc_boundary.h"


#include "devel/misc.h"


namespace odc {

/**
 * 引数解析情報
 */
struct arg_parser_info_t
{
	int parse_count;		/**< 解析実行回数 */
	int parse_error;		/**< 解析エラー(解析処理関数リターン値) */
	arg_parser_info_t() {this->parse_count=0; this->parse_error=0;}
};


/**
 * 引数解析設定
 *
 * <pre>
 * -<name> [<value>]
 * </pre>
 *
 */
template <class T> struct arg_parser_t
{
	const char* name;	/**< 引数名 */
	bool has_value;	/**< 引数が値を持つ場合true */
	int (*parser_func)(char* name, char* value, T* param);	/**< 解析処理関数 */
	arg_parser_info_t* info;	/**< 解析情報格納先ポインタ */
};

/**
 * 引数解析を行う。
 * 戻り値は、解析を終了した引数のインデックス値。最後の引数まで処理した場合は起動引数個数となる。
 * 解析に失敗した場合は負値（解析処理関数のリターン値）を返す。
 *
 *
 * 解析設定は以下のようなデータを設定する。要素の最後は名前がNULLのものを設定すること。
 * <pre>
 * static struct arg_parser_t<param> parser_config[] = {
 * 		{ "パラメータ名",			true,		解析関数,		info	},
 * 			：
 * 		{ NULL,					false,		NULL,			NULL	}
 * 	};
 * 	</pre>
 *
 * @param[in] ac 起動引数個数
 * @param[in] av 起動引数
 * @param[in] cfg 解析設定
 * @param[out] param 解析結果格納先
 * @return 処理結果
 */
template <class T> int arg_parser_parse(int ac, char** av, const arg_parser_t<T>* cfg, T* param)
{
	int i = 1, j, rc;

	for (; i < ac ; i++) {
		if (av[i][0] != '-') {
			// invalid
			return i;
		}

		for (j=0 ; cfg[j].name != NULL ; j++) {
			if (strcmp(&av[i][1], cfg[j].name) == 0) {
				if (cfg[j].has_value) {
					i++;
					if (i == ac) {
						return -1; // has no value
					}
					rc = cfg[j].parser_func(&av[i-1][1], av[i], param);
				} else {
					rc = cfg[j].parser_func(&av[i][1], NULL, param);
				}
				if (cfg[j].info) {
					cfg[j].info->parse_count ++;
				}
				if (rc < 0) {
					if (cfg[j].info) {
						cfg[j].info->parse_error = rc;
					}
					return rc; // parse error
				}
			}
		}
	}
	return i;
}




#define yesno_str(B) (B)?"yes":"no"
int parse_bool_str(const char* s, bool* b);
int parse_usbid(const char* usbid, uint16_t* idVendor, uint16_t* idProduct);


/** Thread Routine Definition */
typedef void* (*thread_routine_t)(void*) ;

/**
 * Threadベースクラス
 */
class thread_base
{
protected:
	pthread_t t;
	pthread_attr_t attr;
	thread_routine_t func;

public:
	thread_base(thread_routine_t f, bool detached = false);
	virtual ~thread_base();
	virtual int start();
	virtual int stop();
	int join();
};

/**
 * Threadクラス
 */
class thread : public thread_base
{
protected:
	static void* run_thread(void* p);

public:
	thread(bool detached = false) : thread_base(thread::run_thread, detached) {}
	virtual ~thread() {}
	virtual int run() = 0;
};




/**
 * Mutex
 */
class mutex
{
protected:
	pthread_mutex_t m;

public:
	mutex()
	{
		pthread_mutex_init(&this->m, NULL);
	}
	virtual ~mutex()
	{
		pthread_mutex_destroy(&this->m);
	}
	void lock()
	{
		pthread_mutex_lock(&this->m);
	}
	void unlock()
	{
		pthread_mutex_unlock(&this->m);
	}
};


/**
 * inter-thread interaction
 *
 * take()は待ち合わせ、give()でシグナル送信し、take()側を作動させる。
 * act_give()、act_take()は、give()、take()での処理内容を派生クラスで実装するための純粋仮想関数。
 * give()は、act_give()を行った後にシグナル送信する。
 * take()はシグナル待ち合わせし、シグナル受信でact_take()を行う。
 *
 */
template <class T>
class interaction : protected mutex
{
protected:
	pthread_cond_t c;

	virtual int act_give(T* p) {return 0;};
	virtual int pre_take(T* p) {return 0;};
	virtual int act_take(T* p) {return 0;};

public:
	interaction()
	{
		pthread_cond_init(&this->c, NULL);
	}
	~interaction()
	{
		pthread_cond_destroy(&this->c);
	}

	int give(T* p=NULL)
	{
		int rc;
		this->lock();
		rc = act_give(p);
		if (rc == 0) {
			pthread_cond_signal(&this->c);
		}
		this->unlock();
		return rc;
	}

	int take(T* p=NULL)
	{
		int rc;
		this->lock();
		rc = pre_take(p);
		if (rc == 0) {
			pthread_cond_wait(&this->c, &this->m);
			rc = act_take(p);
		}
		this->unlock();
		return rc;
	}

	int take(T* p, struct timespec* timeout)
	{
		int rc;
		this->lock();
		rc = pre_take(p);
		if (rc == 0) {
			if ((rc = pthread_cond_timedwait(&this->c, &this->m, timeout)) == 0) {
				rc = act_take(p);
			} else if (rc == ETIMEDOUT) {
			} else {
			}
		}
		this->unlock();
		return rc;
	}
};



/**
 * 排他パラメータの設定メソッド呼び出し時に指定する設定可否テスト関数定義基底クラス
 */
template <class T> class exclusive_param_tester
{
public:
	virtual ~exclusive_param_tester() {}

	/**
	 * 排他パラメータの設定メソッド呼び出し時に指定する設定可否テスト関数
	 * @param[in] nowval 旧値
	 * @param[in] newval 新値
	 * @return 判定結果
	 * @retval true 設定可
	 * @retval false 設定不可
	 */
	virtual bool test(T& nowval, T& newval) = 0;
};


/**
 * 排他パラメータテンプレート
 */
template <class T> class exclusive_param : public mutex
{
protected:
	T param;

public:
	exclusive_param() {}
	exclusive_param(T v) : param(v) {}
	virtual ~exclusive_param() {}

	/**
	 * testerにより設定可能か判定した後に、設定可能なら指定paramを設定します。
	 * @param[in] newval 設定値
	 * @param[out] oldval 旧値（現在値）
	 * @param[in] tester 判定機
	 * @return 設定結果
	 * @retval true 設定できた
	 * @retval false 設定できなかった
	 */
	bool set(T newval, T* oldval, exclusive_param_tester<T>& tester)
	{
		bool result;
		lock();
		*oldval = this->param;
		if ((result = tester.test(this->param, newval))) {
			this->param = newval;
		}
		unlock();
		return result;
	}

	/**
	 * destに値をコピーします。
	 * @param[out] dest 値の格納先
	 */
	void get(T* dest)
	{
		lock();
		*dest = param;
		unlock();
	}
};


}; /* namespace liboscnr */

#endif /* ODC_MISC_H_ */
