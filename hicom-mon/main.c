/*
** Zabbix
** Copyright (C) 2001-2016 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include <zabbix/sysinc.h>
#include <zabbix/module.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include "EBCDICConverter.h"


int sd;

void* server(void *args);
void* session(void* args);

void openSocket()
{
	sd = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(30405);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
		perror("Bind AF_INET");
	if (listen(sd, 20) != 0)
		perror("Listen");
	pthread_t stid;
	if (pthread_create(&stid, 0, server, 0) < 0)
		perror("Server loop");
}

void* session(void* arg)
{
	int clientsd = *((int*)arg);
	char buffer[1024];
	int nbytes;
	do
	{
		nbytes = recv(clientsd, buffer, sizeof(buffer), 0);
		if (nbytes > 0)
			send(clientsd, buffer, nbytes, 0);
	} while (nbytes > 0 && strncmp("bye\r", buffer, 4) != 0);
	close(clientsd);
}

void* server(void* args)
{

	while (true)
	{
		int clientsd;
		struct sockaddr_in addr;
		unsigned int size = sizeof(addr);
		clientsd = accept(sd, (struct sockaddr*)&addr, &size);
		if (clientsd > 0)
		{
			pthread_t client_th;
			pthread_create(&client_th, 0, session, &clientsd);
		}
		else
		{
			perror("Accept");
		}
	}
}


void closeSocket()
{
	close(sd);
}

/* the variable keeps timeout setting for item processing */
static int	item_timeout = 0;

int	zbx_module_dummy_ping(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_dummy_echo(AGENT_REQUEST *request, AGENT_RESULT *result);
int	zbx_module_dummy_random(AGENT_REQUEST *request, AGENT_RESULT *result);

static ZBX_METRIC keys[] =
/*      KEY                     FLAG		FUNCTION        	TEST PARAMETERS */
{
	{ "dummy.ping",		0,		zbx_module_dummy_ping,	NULL },
	{ "dummy.echo",		CF_HAVEPARAMS,	zbx_module_dummy_echo, 	"a message" },
	{ "dummy.random",	CF_HAVEPARAMS,	zbx_module_dummy_random,"1,1000" },
	{ NULL }
};

/******************************************************************************
*                                                                            *
* Function: zbx_module_api_version                                           *
*                                                                            *
* Purpose: returns version number of the module interface                    *
*                                                                            *
* Return value: ZBX_MODULE_API_VERSION_ONE - the only version supported by   *
*               Zabbix currently                                             *
*                                                                            *
******************************************************************************/
int	zbx_module_api_version()
{
	return ZBX_MODULE_API_VERSION_ONE;
}

/******************************************************************************
*                                                                            *
* Function: zbx_module_item_timeout                                          *
*                                                                            *
* Purpose: set timeout value for processing of items                         *
*                                                                            *
* Parameters: timeout - timeout in seconds, 0 - no timeout set               *
*                                                                            *
******************************************************************************/
void	zbx_module_item_timeout(int timeout)
{
	item_timeout = timeout;
}

/******************************************************************************
*                                                                            *
* Function: zbx_module_item_list                                             *
*                                                                            *
* Purpose: returns list of item keys supported by the module                 *
*                                                                            *
* Return value: list of item keys                                            *
*                                                                            *
******************************************************************************/
ZBX_METRIC	*zbx_module_item_list()
{
	return keys;
}

int	zbx_module_dummy_ping(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	SET_UI64_RESULT(result, 1);

	return SYSINFO_RET_OK;
}

int	zbx_module_dummy_echo(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	char	*param;

	if (1 != request->nparam)
	{
		/* set optional error message */
		SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
		return SYSINFO_RET_FAIL;
	}

	param = get_rparam(request, 0);

	SET_STR_RESULT(result, strdup(param));

	return SYSINFO_RET_OK;
}

/******************************************************************************
*                                                                            *
* Function: zbx_module_dummy_random                                          *
*                                                                            *
* Purpose: a main entry point for processing of an item                      *
*                                                                            *
* Parameters: request - structure that contains item key and parameters      *
*              request->key - item key without parameters                    *
*              request->nparam - number of parameters                        *
*              request->timeout - processing should not take longer than     *
*                                 this number of seconds                     *
*              request->params[N-1] - pointers to item key parameters        *
*                                                                            *
*             result - structure that will contain result                    *
*                                                                            *
* Return value: SYSINFO_RET_FAIL - function failed, item will be marked      *
*                                 as not supported by zabbix                 *
*               SYSINFO_RET_OK - success                                     *
*                                                                            *
* Comment: get_rparam(request, N-1) can be used to get a pointer to the Nth  *
*          parameter starting from 0 (first parameter). Make sure it exists  *
*          by checking value of request->nparam.                             *
*                                                                            *
******************************************************************************/
int	zbx_module_dummy_random(AGENT_REQUEST *request, AGENT_RESULT *result)
{
	char	*param1, *param2;
	int	from, to;

	if (2 != request->nparam)
	{
		/* set optional error message */
		SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
		return SYSINFO_RET_FAIL;
	}

	param1 = get_rparam(request, 0);
	param2 = get_rparam(request, 1);

	/* there is no strict validation of parameters for simplicity sake */
	from = atoi(param1);
	to = atoi(param2);

	if (from > to)
	{
		SET_MSG_RESULT(result, strdup("Invalid range specified."));
		return SYSINFO_RET_FAIL;
	}

	SET_UI64_RESULT(result, from + rand() % (to - from + 1));

	return SYSINFO_RET_OK;
}

/******************************************************************************
*                                                                            *
* Function: zbx_module_init                                                  *
*                                                                            *
* Purpose: the function is called on agent startup                           *
*          It should be used to call any initialization routines             *
*                                                                            *
* Return value: ZBX_MODULE_OK - success                                      *
*               ZBX_MODULE_FAIL - module initialization failed               *
*                                                                            *
* Comment: the module won't be loaded in case of ZBX_MODULE_FAIL             *
*                                                                            *
******************************************************************************/
int	zbx_module_init()
{
	/* initialization for dummy.random */
	srand(time(NULL));
	openSocket();
	return ZBX_MODULE_OK;
}

/******************************************************************************
*                                                                            *
* Function: zbx_module_uninit                                                *
*                                                                            *
* Purpose: the function is called on agent shutdown                          *
*          It should be used to cleanup used resources if there are any      *
*                                                                            *
* Return value: ZBX_MODULE_OK - success                                      *
*               ZBX_MODULE_FAIL - function failed                            *
*                                                                            *
******************************************************************************/
int	zbx_module_uninit()
{
	closeSocket();
	return ZBX_MODULE_OK;
}
