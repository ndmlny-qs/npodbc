#include <Python.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>

#include <sstream>
#include <string>

// #include "object.h"

static PyObject *list_drivers(PyObject *self) {
    SQLHENV env;
    char driver[256];
    char attr[256];
    std::string ret_str;
    SQLSMALLINT driver_ret;
    SQLSMALLINT attr_ret;
    SQLUSMALLINT direction;
    SQLRETURN ret;

    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);

    direction = SQL_FETCH_FIRST;
    while (SQL_SUCCEEDED(
            ret = SQLDrivers(
                    env, direction, (SQLCHAR *)driver, sizeof(driver), &driver_ret,
                    (SQLCHAR *)attr, sizeof(attr), &attr_ret
            )
    )) {
        direction = SQL_FETCH_NEXT;
        if (ret == SQL_SUCCESS_WITH_INFO) {
            ret_str.append("\tdata truncated");
        } else {
            ret_str.append(std::string(driver) + " - " + std::string(attr) + "\n");
        }
    }
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return PyUnicode_FromString(ret_str.c_str());
}

static PyObject *list_data_sources(PyObject *self) {
    SQLHENV env = SQL_NULL_HANDLE;
    char dsn[256];
    char desc[256];
    SQLSMALLINT dsn_ret;
    SQLSMALLINT desc_ret;
    SQLUSMALLINT direction;
    SQLRETURN ret;
    std::string ret_str;

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);

    direction = SQL_FETCH_NEXT;
    while (SQL_SUCCEEDED(
            ret = SQLDataSources(
                    env, direction, (SQLCHAR *)dsn, sizeof(dsn), &dsn_ret,
                    (SQLCHAR *)desc, sizeof(desc), &desc_ret
            )
    )) {
        direction = SQL_FETCH_NEXT;
        if (ret == SQL_SUCCESS_WITH_INFO) {
            ret_str.append("\tdata truncation");
        } else {
            ret_str.append(std::string(dsn) + " - " + std::string(desc) + "\n");
        }
    }
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return PyUnicode_FromString(ret_str.c_str());
}

std::string extract_error(char *fn, SQLHANDLE handle, SQLSMALLINT type) {
    SQLINTEGER i = 0;
    SQLINTEGER NativeError;
    SQLCHAR SQLState[7];
    SQLCHAR MessageText[256];
    SQLSMALLINT TextLength;
    SQLRETURN ret;
    std::string ret_str;
    std::ostringstream oss;

    do {
        ret = SQLGetDiagRec(
                type, handle, ++i, SQLState, &NativeError, MessageText,
                sizeof(MessageText), &TextLength
        );
        if (SQL_SUCCEEDED(ret)) {
            oss << SQLState << ":" << long(i) << ":" << long(NativeError) << ":"
                << MessageText << "\n";
            ret_str.append(oss.str());
        }
    } while (ret == SQL_SUCCESS);
    return ret_str;
}

static PyObject *driver_connect(PyObject *self) {
    SQLHENV env;
    SQLHDBC dbc;
    SQLRETURN ret;
    char outstr[1024];
    SQLSMALLINT outstrlen;
    std::string ret_str;

    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    ret = SQLDriverConnect(
            dbc, NULL, (SQLCHAR *)"DSN=mssql2022;UID=SA;PWD=StrongPassword2022!",
            SQL_NTS, (SQLCHAR *)outstr, sizeof(outstr), &outstrlen, SQL_DRIVER_NOPROMPT
    );

    if (SQL_SUCCEEDED(ret)) {
        ret_str.append("Connected!\n");
        ret_str.append("Connection string:\n\t" + std::string(outstr) + "\n\n");
        if (ret == SQL_SUCCESS_WITH_INFO) {
            ret_str.append(
                    "Driver reported the following diagnostics\n" +
                    extract_error((char *)"SQLDriverConnect", dbc, SQL_HANDLE_DBC) +
                    "\n"
            );
        }
        SQLDisconnect(dbc);
    } else {
        ret_str.append(
                "Failed to connect\n" +
                extract_error((char *)"SQLDriverConnect", dbc, SQL_HANDLE_DBC)
        );
    }

    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, env);
    return PyUnicode_FromString(ret_str.c_str());
}

static PyMethodDef methods[] = {
        {"list_drivers", (PyCFunction)list_drivers, METH_NOARGS, NULL},
        {"list_data_sources", (PyCFunction)list_data_sources, METH_NOARGS, NULL},
        {"driver_connect", (PyCFunction)driver_connect, METH_NOARGS, NULL},
        {NULL, NULL, 0, NULL},
};

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT, "npodbc", NULL, -1, methods, NULL, NULL, NULL, NULL,
};

PyMODINIT_FUNC PyInit_npodbc(void) {
    PyObject *module = PyModule_Create(&moduledef);
    if (!module) {
        return NULL;
    }
    return module;
}
