#!/bin/bash
USER="$1"
PASSWORD="$2"
DIR="$3"
EVENT="$4"

CREATE="mio_node_create"
REFERENCE="mio_reference_child_add"
METAADD="mio_meta_add"
METACOPY="meta_publish"

AUTH_FLAG="-u $USER -p $PASSWORD"

declare -A ids
# create template folder
TEMPLATE_UUID=$(uuidgen)
ids[$DIR]=$TEMPLATE_UUID
$CREATE $AUTH_FLAG -event $TEMPLATE_UUID -title "template" 
$METAADD $AUTH_FLAG -event $TEMPLATE_UUID -type location -name templates
$REFERENCE $AUTH_FLAG -child $TEMPLATE_UUID -parent $EVENT -add_ref_child

FOLDERS=($(ls -d $DIR/*))
index=0
while [ ${#FOLDERS[@]} -gt 0 ] ;
do
  FOLDER=${FOLDERS[$index]}
  unset FOLDERS[$index]
  TMP_UUID=$(uuidgen)
  if [[ $FOLDER != *"xml" ]] ;
    then
    echo "$FOLDER"
    if [[ $FOLDER == "" ]] ; 
    then
    continue
    fi
    ids[$FOLDER]="$TMP_UUID"
    PARENT=$(dirname $FOLDER)
    echo "$PARENT PARENT $FOLDER $(dirname $FOLDER)"
    TITLE=$(basename $FOLDER)
    PARENT=${ids[$PARENT]}
    FOLDERS+=($(ls -d $FOLDER/*))
    if [[ $? > 0 ]] ; 
      then
      index=$index+1
      continue;
    fi
    $CREATE $AUTH_FLAG -event $TMP_UUID -title $TITLE
    echo "$METAADD $AUTH_FLAG -event $TMP_UUID -type location -name $TITLE"
    $METAADD $AUTH_FLAG -event $TMP_UUID -type location -name $TITLE
    echo "$REFERENCE $AUTH_FLAG -parent $PARENT -child $TMP_UUID -add_ref_child"
    $REFERENCE $AUTH_FLAG -parent $PARENT -child $TMP_UUID -add_ref_child
  else
    ids[$FOLDER]=$TMP_UUID
    PARENT=$(dirname $FOLDER)
    PARENT=${ids[$PARENT]}
    name=$(basename $FOLDER)
    $CREATE $AUTH_FLAG -event $TMP_UUID -title $name
    #$METAADD $AUTH_FLAG -event $TMP_UUID -type device -name $name -overwrite
    echo "$METACOPY $AUTH_FLAG -id $TMP_UUID -path $FOLDER"
    $METACOPY $AUTH_FLAG -id $TMP_UUID -path $FOLDER -overwrite
    echo "$REFERENCE $AUTH_FLAG -parent $PARENT -child $TMP_UUID -add_ref_child"
    $REFERENCE $AUTH_FLAG -parent $PARENT -child $TMP_UUID -add_ref_child
  fi
  index=$index+1
done
