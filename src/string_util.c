#include "string_util.h"

/*
 * 从字符串中解析key对应的value。
 * 字符串类似: key1|value1,key2|value2, d1对应',', d2对应'|'
 */
void get_value(const char *str, char *d1, char *d2, const char *key, char *value, size_t value_size) {
	value[0] = '\0';
	size_t str_size = strlen(str);
	char *tmp = (char *)calloc(str_size+1, sizeof(char));
	memcpy(tmp, str, str_size+1);

	char *pch;
	pch = strtok(tmp, d1);

	while (pch != NULL) {
		if (memcmp(pch, key, strlen(key)) == 0 && 
		   memcmp(pch+strlen(key), d2, strlen(d2))==0) {
			break;	
		}
		pch = strtok(NULL, d1);
	}

	// 如果找到指定参数则提取=号后面的值，并拷贝到value指向的存储空间
	if (pch != NULL) {
		// 找到了
		char *pch1;
		pch1 = strtok(pch, d2);
		pch1 = strtok(NULL, d2);
		if (pch1 != NULL) {
			strncpy(value, pch1, value_size-1);
			value[value_size-1] = '\0';
		} 
	}

	free(tmp);
}

/*
 * 解析get请求中参数值
 * 参数说明: 
 * query:		get请求中携带的参数，如:a=1&b=2&c=3
 * key:			希望获取的参数名，如: b
 * value:		用于存储参数key对应的值
 * value_size:	value指针指向的内存空间大小
 */
void get_query_value(const char *query, const char *key, char *value, size_t value_size) {
	value[0] = '\0';
	if(query == NULL || key == NULL || value == NULL || value_size <= 0) return;
	
	get_value(query, "&", "=",  key, value, value_size);
	return;
}
