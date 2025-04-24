/*
 * odc_misc.cpp
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "odc_misc.h"

#include "odc_tracer.h"
#define TRCID "misc"
#define TRACE(LEVEL, ERRNO, ...) T_TRACE((LEVEL), TRCID, ERRNO, __VA_ARGS__)
#define FATAL(ERRNO, ...) T_FATAL(TRCID, ERRNO, __VA_ARGS__)
#define ERR(ERRNO, ...) T_ERR(TRCID, ERRNO, __VA_ARGS__)
#define WRN(ERRNO, ...) T_WARN(TRCID, ERRNO, __VA_ARGS__)
#define INFOh(...) T_INFO_H(TRCID, __VA_ARGS__)
#define INFO(...) T_INFO(TRCID, __VA_ARGS__)
#define INFOl(...) T_INFO_L(TRCID, __VA_ARGS__)
#define MESSAGE(...) T_MESSAGE(TRCID, __VA_ARGS__)
#define DBG(...) T_DBG(TRCID, __VA_ARGS__)
#define TRC(...) T_TRC1(TRCID, __VA_ARGS__)
#define TRC2(...) T_TRC2(TRCID, __VA_ARGS__)
#define TRC3(...) T_TRC3(TRCID, __VA_ARGS__)
#define TRC4(...) T_TRC4(TRCID, __VA_ARGS__)
#define TMP(...) T_TMP(TRCID, 0, __VA_ARGS__)
#define DUMP(DATA, SIZE) T_DUMP(TRCID, DATA, SIZE)

namespace odc {

/**
 * ピクセル数計算マクロ
 * 指定のミリメートル値とDPIから、ピクセル数を求めます。
 * @param[in] dpi DPI
 * @param[in] mm ミリメートル値
 * @return ピクセル値
 */
int mm2pixel(int dpi, int mm)
{
	return static_cast<int>((static_cast<double>(dpi) * static_cast<double>(mm)) / static_cast<double>(MM_INCH));
}

/**
 * ピクセル数計算マクロ
 * 指定のミリメートル値とDPIから、ピクセル数を求めます。
 * @param[in] dpi DPI
 * @param[in] mm ミリメートル値
 * @return ピクセル値
 */
int mm2pixel(double dpi, double mm)
{
	return static_cast<int>((dpi * mm) / static_cast<double>(MM_INCH));
}


/**
 * LibStatus文字列化
 * @param[in] status LibStatus
 * @return ステータスの内容を示す文字列
 */
const char* strlibstatus(LibStatus status)
{
	switch (status) {
	case SUCCESS:					return DRV_I18N("Success.");

	case CANCELED:					return DRV_I18N("Canceled.");
	case END_OF_PAGE:				return DRV_I18N("End of page.");
	case HAS_MORE_PAGE:				return DRV_I18N("Has more pages.");
	case NO_MORE_PAGE:				return DRV_I18N("No more page.");
	case NODATA:					return DRV_I18N("No data.");

	case CONNECTED:					return DRV_I18N("Already Connected.");
	case NOT_CONNECTED:				return DRV_I18N("Not Connected.");
	case DISCONNECTED:				return DRV_I18N("Disconnected.");
	case DISCONNECT:				return DRV_I18N("Disconnect.");

	case ERROR:						return DRV_I18N("Error.");
	case FAIL:						return DRV_I18N("Fail.");
	case ARRAY_INDEX_OUT_OF_RANGE:	return DRV_I18N("Array index out of range.");
	case ILLSTATE:					return DRV_I18N("Illegal state.");
	case INVARG:					return DRV_I18N("Invalid argument.");
	case INVVAL:					return DRV_I18N("Invalid value.");
	case INVPRM:					return DRV_I18N("Invalid parameter.");
	case INVFNCCALL:				return DRV_I18N("Invalid function call.");
	case INVDATA:					return DRV_I18N("Invalid data.");
	case INVALID:					return DRV_I18N("Invalid.");
	case TIMEOUT:					return DRV_I18N("Timeout.");
	case LOCKED:					return DRV_I18N("Locked.");
	case BUSY:						return DRV_I18N("Busy.");
	case REJECT:					return DRV_I18N("Reject.");
	case IOERROR:					return DRV_I18N("I/O error.");
	case NOTENOSPC:					return DRV_I18N("Not enough space.");
	case NOMEMORY:					return DRV_I18N("Not enough memory.");
	case UNKNOWN_USER:				return DRV_I18N("Unknown user.");
	case NOT_READY:					return DRV_I18N("Not ready.");
	case NOT_STANDBY:				return DRV_I18N("Not standby.");
	case NOT_SUPPORTED:				return DRV_I18N("Not supported.");
	case NOT_EXISTS:				return DRV_I18N("Not exists.");

	case CONNECTION_REFUSED:		return DRV_I18N("Connection refused.");

	case FIOERROR:					return DRV_I18N("File I/O error.");
	case OPEN_ERROR:				return DRV_I18N("Open error.");
	case ALREADY_OPENED:			return DRV_I18N("Already opened.");
	case NOT_OPENED:				return DRV_I18N("Not opened.");
	case CLOSED:					return DRV_I18N("Closed.");
	case NO_SUCH_FILE:				return DRV_I18N("No such file.");
	case PERMISSION_DENIED:			return DRV_I18N("Permission denied.");
	case END_OF_FILE:				return DRV_I18N("End of file.");
	case HAS_NO_ENTRIES:			return DRV_I18N("Has no entries.");
	case HAS_ENTRIES:				return DRV_I18N("Has entries.");
	case UNKNOWN_DEVICE:			return DRV_I18N("Unknown device.");
	case UNKNOWN_MODEL:				return DRV_I18N("Unknown model.");


	/* scanner status */
	case DEVICE_BUSY:				return DRV_I18N("Scanner Error (Scanner is busy)");
	case SYSTEM_BUSY:				return DRV_I18N("Scanner Error (System busy)");
	case ACCESS_REFUSED:			return DRV_I18N("Scanner Error (Runtime Error)");
	case ACCESS_DENIED:				return DRV_I18N("Scanner Error (Rejected)");
	case ADF_NO_PAPER:				return DRV_I18N("Scanner Error (ADF No Paper)");
	case ADF_PAPER_COVER_OPEN:		return DRV_I18N("Scanner Error (ADF Paper Cover Open)");
	case ADF_PAPER_JAMMED:			return DRV_I18N("Scanner Error (ADF Paper Jam)");
	case ADF_PAPER_FEED_ERROR:		return DRV_I18N("Scanner Error (ADF Paper Feed Error)");
	case CCD_CARRIAGE_LOCK:			return DRV_I18N("Scanner Error (Check CCD carriage lock)");
	case HOME_POSITION_ERROR:		return DRV_I18N("Scanner Error (Home Position Error)");
	case MEMORY_FULL:				return DRV_I18N("Scanner Error (Memory Full)");
	case INVALID_COMMAND_CODE:		return DRV_I18N("Scanner Error (Invalid Command Code)");
	case INVALID_VALUE:				return DRV_I18N("Scanner Error (Invalid Value)");
	case INVALID_PARAMETER:			return DRV_I18N("Scanner Error (Invalid Parameter)");
	case INVALID_DATA:				return DRV_I18N("Scanner Error (Invalid Data)");
	case INVALID_COMMAND_LENGTH:	return DRV_I18N("Scanner Error (Command Length Error)");
	case INVALID_DATA_LENGTH:		return DRV_I18N("Scanner Error (Data Length Error)");
	case RAM_TEST_FAILED:			return DRV_I18N("Scanner Error (Scanner RAM test failed)");
	case CALIBRATION_TEST_FAILED:	return DRV_I18N("Scanner Error (Calibration test failed)");
	case DEVICE_INTERNAL_ERROR:		return DRV_I18N("Scanner Error (Scanner Internal Error)");
	case SENSOR_ERROR:				return DRV_I18N("Scanner Error (Sensor Error)");
	case HOME_SENSOR_ERROR:			return DRV_I18N("Scanner Error (Home sensor is not found)");
	case LAMP_ERROR:				return DRV_I18N("Scanner Error (Lamp Error)");
	case FAN_ERROR:					return DRV_I18N("Scanner Error (Fan Lock Error)");
	case UNPLUGGED_ERROR:			return DRV_I18N("Scanner Error (Scanner unplugged)");
	case MIRROR_CARRIAGE_ERROR:		return DRV_I18N("Scanner Error (Mirror Carriage Error)");
	case COMMUNICATION_ERROR:		return DRV_I18N("Scanner Error (Communication Error)");
	case DETECT_CD:					return DRV_I18N("Scanner Error (Pattern recognition Error)");

	default:	break;
	}
	return DRV_I18N("Unknown status.");
}


/**
 * ブール値を表す文字列の判定を行います。
 * 指定された文字列が"true","yes","on"の場合、trueを返します。
 * 指定された文字列が"false","no","off"の場合、falseを返します。
 * 大文字小文字の区別は行いません。
 * 判定が正しく行われた場合（正しい文字列の場合）は0を返します。
 * 不正な場合は負値を返します。
 * @param[in] s 文字列
 * @param[out] b 判定結果
 * @return 処理結果
 */
int parse_bool_str(const char* s, bool* b)
{
	if (s) {
		if (strcasecmp("yes", s) == 0 ||
			strcasecmp("true", s) == 0||
			strcasecmp("on", s) == 0) {
			*b = true;
			return 0;
		} else if (strcasecmp("no", s) == 0 ||
			strcasecmp("false", s) == 0||
			strcasecmp("off", s) == 0) {
			*b = false;
			return 0;
		}
	}
	return -1;
}


/**
 * USB識別文字列解析
 * hhhh:hhhhの形式の文字列から、idVendorとidProductの各値に変換する。
 * @param[in] usbid USB識別文字列
 * @param[out] idVendor idVendor値格納先ポインタ
 * @param[out] idProduct idProduct値格納先ポインタ
 * @return 処理結果
 * @retval 0 成功
 * @retval -1 処理失敗（形式不正）
 */
int parse_usbid(const char* usbid, uint16_t* idVendor, uint16_t* idProduct)
{
	int i;
	uint16_t* p;
	*idVendor = 0;
	*idProduct = 0;

	for (i=0, p=idVendor ; usbid[i] != '\0' ; i++)
	{
		switch(usbid[i])
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			*p = (*p << 4) + (usbid[i] - '0');
			break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			*p = (*p << 4) + (usbid[i] - 'a' + 10);
			break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			*p = (*p << 4) + (usbid[i] - 'A' + 10);
			break;
		case ':':
			if (p == idProduct)
			{
				return -1;
			}
			p = idProduct;
			break;
		default:
			return -1;
		}
	}
	if (i != 9)
	{
		return -1;
	}
	return 0;
}





/**
 * threadを初期化します。
 */
thread_base::thread_base(thread_routine_t f, bool detached)
{
	int rc;

	this->func = f;

    rc = pthread_attr_init(&this->attr);
    if (rc < 0) {
        ERR(errno, "pthread_addr_init");
    }

    if (detached) {
		rc = pthread_attr_setdetachstate(&this->attr, PTHREAD_CREATE_DETACHED);
		if (rc < 0) {
			ERR(errno, "pthread_addr_init");
		}
    }
}


/**
 * threadを破棄します。
 */
thread_base::~thread_base()
{
	stop();
}


/**
 * スレッドを起動します。
 * @return 処理結果
 * @retval 0 成功
 * @retval 負値 失敗
 */
int thread_base::start()
{
	int rc;

	rc = pthread_create(&this->t, &this->attr, this->func, this);
    if (rc < 0) {
        ERR(errno, "pthread_create");
        return -1;
    }

    return 0;
}


/**
 * スレッドを停止します。
 * 基底クラスでの実装はありません。
 * 派生クラスで実装を行ってください。
 * @return 処理結果
 * @retval 0 成功
 * @retval 負値 失敗
 */
int thread_base::stop()
{
	return 0;
}


/**
 * スレッドの終了を待ちます。
 * @return 処理結果
 * @retval 0 成功
 * @retval 負値 失敗
 */
int thread_base::join()
{
	DBG("pthread_join(%lx) start...", this->t);
	int rc = pthread_join(this->t, NULL);
	if (rc != 0) {
		ERR(errno, "pthread_join error. (id=%lx)", this->t);
	}
	DBG("pthread_join(%lx) end.", this->t);
	return rc;
}





/**
 * run()メソッドを実行します。
 * @param[in] p threadインスタンスへのポインタ
 * @return NULL
 */
void* thread::run_thread(void* p)
{
	thread* t = static_cast<thread*>(p);
	int rc;
	rc = t->run();
	TRC("rc=%d", rc);
	return NULL;
}




}; /* namespace libodc */
