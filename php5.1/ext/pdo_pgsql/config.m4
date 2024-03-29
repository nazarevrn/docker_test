dnl
dnl $Id$
dnl

if test "$PHP_PDO" != "no"; then

AC_DEFUN([PHP_PGSQL_CHECK_FUNCTIONS],[
])

PHP_ARG_WITH(pdo-pgsql,for PostgreSQL support for PDO,
[  --with-pdo-pgsql[=DIR]    PDO: PostgreSQL support.  DIR is the PostgreSQL base
                            install directory or the path to pg_config])

if test "$PHP_PDO_PGSQL" != "no"; then
  PHP_EXPAND_PATH($PGSQL_INCLUDE, PGSQL_INCLUDE)

  AC_MSG_CHECKING(for pg_config)
  for i in $PHP_PDO_PGSQL $PHP_PDO_PGSQL/bin /usr/local/pgsql/bin /usr/local/bin /usr/bin ""; do
	if test -x $i/pg_config; then
      PG_CONFIG="$i/pg_config"
      break;
    fi
  done

  if test -n "$PG_CONFIG"; then
    AC_MSG_RESULT([$PG_CONFIG])
    PGSQL_INCLUDE=`$PG_CONFIG --includedir`
    PGSQL_LIBDIR=`$PG_CONFIG --libdir`
    AC_DEFINE(HAVE_PG_CONFIG_H,1,[Whether to have pg_config.h])
  else
    AC_MSG_RESULT(not found)
    if test "$PHP_PDO_PGSQL" = "yes"; then
      PGSQL_SEARCH_PATHS="/usr /usr/local /usr/local/pgsql"
    else
      PGSQL_SEARCH_PATHS=$PHP_PDO_PGSQL
    fi
  
    for i in $PGSQL_SEARCH_PATHS; do
      for j in include include/pgsql include/postgres include/postgresql ""; do
        if test -r "$i/$j/libpq-fe.h"; then
          PGSQL_INC_BASE=$i
          PGSQL_INCLUDE=$i/$j
          if test -r "$i/$j/pg_config.h"; then
            AC_DEFINE(HAVE_PG_CONFIG_H,1,[Whether to have pg_config.h])
          fi
        fi
      done

      for j in lib lib/pgsql lib/postgres lib/postgresql ""; do
        if test -f "$i/$j/libpq.so" || test -f "$i/$j/libpq.a"; then 
          PGSQL_LIBDIR=$i/$j
        fi
      done
    done
  fi

  if test -z "$PGSQL_INCLUDE"; then
    AC_MSG_ERROR(Cannot find libpq-fe.h. Please specify correct PostgreSQL installation path)
  fi

  if test -z "$PGSQL_LIBDIR"; then
    AC_MSG_ERROR(Cannot find libpq.so. Please specify correct PostgreSQL installation path)
  fi

  if test -z "$PGSQL_INCLUDE" -a -z "$PGSQL_LIBDIR" ; then
    AC_MSG_ERROR([Unable to find libpq anywhere under $withval])
  fi

  AC_DEFINE(HAVE_PDO_PGSQL,1,[Whether to build PostgreSQL for PDO support or not])

  AC_MSG_CHECKING([for openssl dependencies])
  if grep -q openssl $PGSQL_INCLUDE/libpq-fe.h ; then
	 AC_MSG_RESULT([yes])
	 if pkg-config openssl ; then
      PDO_PGSQL_CFLAGS="`pkg-config openssl --cflags`"
    fi
  else
	 AC_MSG_RESULT([no])
  fi

  old_LIBS=$LIBS
  old_LDFLAGS=$LDFLAGS
  LDFLAGS="$LDFLAGS -L$PGSQL_LIBDIR"
  AC_CHECK_LIB(pq, PQescapeString,AC_DEFINE(HAVE_PQESCAPE,1,[PostgreSQL 7.2.0 or later]))
  AC_CHECK_LIB(pq, PQsetnonblocking,AC_DEFINE(HAVE_PQSETNONBLOCKING,1,[PostgreSQL 7.0.x or later]))
  AC_CHECK_LIB(pq, PQcmdTuples,AC_DEFINE(HAVE_PQCMDTUPLES,1,[Broken libpq under windows]))
  AC_CHECK_LIB(pq, PQoidValue,AC_DEFINE(HAVE_PQOIDVALUE,1,[Older PostgreSQL]))
  AC_CHECK_LIB(pq, PQclientEncoding,AC_DEFINE(HAVE_PQCLIENTENCODING,1,[PostgreSQL 7.0.x or later]))
  AC_CHECK_LIB(pq, PQparameterStatus,AC_DEFINE(HAVE_PQPARAMETERSTATUS,1,[PostgreSQL 7.4 or later]))
  AC_CHECK_LIB(pq, PQprotocolVersion,AC_DEFINE(HAVE_PQPROTOCOLVERSION,1,[PostgreSQL 7.4 or later]))
  AC_CHECK_LIB(pq, PQtransactionStatus,AC_DEFINE(HAVE_PGTRANSACTIONSTATUS,1,[PostgreSQL 7.4 or later]))
  AC_CHECK_LIB(pq, PQunescapeBytea,AC_DEFINE(HAVE_PQUNESCAPEBYTEA,1,[PostgreSQL 7.4 or later]))
  AC_CHECK_LIB(pq, PQExecParams,AC_DEFINE(HAVE_PQEXECPARAMS,1,[PostgreSQL 7.4 or later]))
  AC_CHECK_LIB(pq, PQresultErrorField,AC_DEFINE(HAVE_PQRESULTERRORFIELD,1,[PostgreSQL 7.4 or later]))
  AC_CHECK_LIB(pq, pg_encoding_to_char,AC_DEFINE(HAVE_PGSQL_WITH_MULTIBYTE_SUPPORT,1,[Whether libpq is compiled with --enable-multibyte]))
  
  AC_CHECK_LIB(pq, PQprepare,AC_DEFINE(HAVE_PQPREPARE,1,[prepared statements]))

  LIBS=$old_LIBS
  LDFLAGS=$old_LDFLAGS

  PHP_ADD_LIBRARY_WITH_PATH(pq, $PGSQL_LIBDIR, PDO_PGSQL_SHARED_LIBADD)
  PHP_SUBST(PDO_PGSQL_SHARED_LIBADD)

  PHP_ADD_INCLUDE($PGSQL_INCLUDE)

  ifdef([PHP_CHECK_PDO_INCLUDES],
  [
  	PHP_CHECK_PDO_INCLUDES
  ],[
    AC_MSG_CHECKING([for PDO includes])
    if test -f $abs_srcdir/include/php/ext/pdo/php_pdo_driver.h; then
      pdo_inc_path=$abs_srcdir/ext
    elif test -f $abs_srcdir/ext/pdo/php_pdo_driver.h; then
      pdo_inc_path=$abs_srcdir/ext
    elif test -f $prefix/include/php/ext/pdo/php_pdo_driver.h; then
      pdo_inc_path=$prefix/include/php/ext
    else
      AC_MSG_ERROR([Cannot find php_pdo_driver.h.])
    fi
    AC_MSG_RESULT($pdo_inc_path)
  ])

  PHP_NEW_EXTENSION(pdo_pgsql, pdo_pgsql.c pgsql_driver.c pgsql_statement.c, $ext_shared,,-I$pdo_inc_path $PDO_PGSQL_CFLAGS)
  ifdef([PHP_ADD_EXTENSION_DEP],
  [
    PHP_ADD_EXTENSION_DEP(pdo_pgsql, pdo) 
  ])
fi

fi
