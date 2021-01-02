#!/bin/sh
# w3mdict.cgi - A dictd dictionary query cgi for w3m
#
# REQUIREMENTS:
# + dict client software
# + an address of a dict server, for variable ${DICT_SERVER}
# + a name of a favorite database on that server, for variable
#   ${FAVORITE_DATABASE}
# OPTIONALLY:
# + locally install a dict server (eg. dictd) and a collection
#   of dict databases (eg. wordnet, aka "wn")

DICT_SERVER="localhost"
FAVORITE_DATABASE="wn"
RETURN_MESSAGE="\n\nPress 'B' to return to the previous page."
printf "Content-type: text/plain\n"
type dict \
|| {
  # Originally, we inconsiderately failed silently ...
  #     printf "W3m-control: BACK\n\n"
  printf "\n\nERROR: dict client software not found${RETURN_MESSAGE}"
  exit
  }
# First, we check only our best and favorite database. This is most
# likely to give us a best defintion, and avoids displaying a long and
# cluttered page with entries from many databases.
dict --host "${DICT_SERVER}" \
     --database "${FAVORITE_DATABASE}" \
     "${QUERY_STRING}" 2>&1 \
&& {
  printf "${RETURN_MESSAGE}"
  } \
|| {
  # The initial attempt failed, so let's search ALL databases
  # available on the server.
  dict --host "${DICT_SERVER}" \
       "${QUERY_STRING}" 2>&1 \
  && {
    printf "${RETURN_MESSAGE}"
    } \
  || {
    # No defintions were found in any of the server's databases, so
    # let's return to the favorite database in order to retrieve its
    # guess of what we meant to type. Originally, for this case, we
    # pushed the user's default action to be entering another word for
    # a dict defintion, so the print command was:
    #     printf "W3m-control: DICT_WORD\n\n"
    # Now, we need only print a blank line to separate the cgi header
    # from the page content.
    printf "\n"
    dict --host "${DICT_SERVER}" \
         --database "${FAVORITE_DATABASE}" \
         "${QUERY_STRING}" 2>&1
    printf "${RETURN_MESSAGE}"
    }
  }
