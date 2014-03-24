<?php

/**
 * @file
 * This file contains helper functions used throughout this module.
 */

/**
 * Gets the primary investigator of the given object.
 *
 * @param AbstractObject $object
 *   The object to get the primary investigator of.
 *
 * @return bool
 *   TRUE if the user is the primary investigator, FALSE otherwise.
 */
function saskdora_get_primary_investigator(AbstractObject $object) {
  return $object->owner;
}

/**
 * Checks if the user is the primary investigator of the given object.
 *
 * @param AbstractObject $object
 *   The object to check against.
 * @param $account
 *   (optional) The account to check, if not given use currently logged in user.
 *
 * @return bool
 *   TRUE if the user is the primary investigator, FALSE otherwise.
 */
function saskdora_is_primary_investigator(AbstractObject $object, $account = NULL) {
  global $user;
  $account = isset($account) ? $account : $user;
  return $object->owner == $account->name;
}

/**
 * Checks if the user is a collaborator of the given object.
 *
 * @param AbstractObject $object
 *   The object to check against.
 * @param $account
 *   (optional) The account to check, if not given use currently logged in user.
 *
 * @return bool
 *   TRUE if the user is a collaborator, FALSE otherwise.
 */
function saskdora_is_collaborator(AbstractObject $object, $account = NULL) {
  global $user;
  $account = isset($account) ? $account : $user;
  return in_array($account->name, saskdora_get_collaborators($object));
}

/**
 * Gets the list of collaborators of the given object.
 *
 * @param AbstractObject $object
 *   The object whose collaborators will be returned.
 *
 * @return string[]
 *   The names of the collaborators.
 */
function saskdora_get_collaborators(AbstractObject $object) {
  if (isset($object['POLICY'])) {
    $xacml = new Xacml($object['POLICY']->content);
    if ($xacml->managementRule->isPopulated()) {
      return $xacml->managementRule->getUsers();
    }
  }
  return array();
}

/**
 * Sets the object's collaborators.
 *
 * @throws Exception
 *   If the object's POLICY datastream can't be modified or created.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param string[] $collaborators
 *   The collaborators to set on the object.
 */
function saskdora_set_collaborators(AbstractObject $object, array $collaborators) {
  $xacml = new Xacml();
  $xacml->managementRule->addUser(saskdora_get_primary_investigator($object));
  $xacml->managementRule->addUser($collaborators);
  if (isset($object['POLICY'])) {
    try {
      $object['POLICY']->content = $xacml->getXmlString();
    } catch (Exception $e) {
      $msg = t('Could not update the POLICY datastream on Object %pid .', array('%pid' => $object->id));
      throw new Exception($msg, $e->getCode(), $e);
    }
  }
  else {
    try {
      $ds = $object->constructDatastream('POLICY');
      $ds->mimetype = 'text/xml';
      $ds->content = $xacml->getXmlString();
      $object->ingestDatastream($ds);
    } catch (Exception $e) {
      $msg = t('Could not create the POLICY datastream on Object %pid .', array('%pid' => $object->id));
      throw new Exception($msg, $e->getCode(), $e);
    }
  }
}

/**
 * Updates the object's collaborators.
 *
 * @throws Exception
 *   If the object's POLICY datastream can't be modified or created.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param string[] $add
 *   The collaborators to add to the object.
 * @param string[] $remove
 *   The collaborators to remove from the object.
 */
function saskdora_update_collaborators(AbstractObject $object, array $add, array $remove) {
  if (!isset($object['POLICY'])) {
    saskdora_set_collaborators($object, $add);
  }
  else {
    try {
      $xacml = new Xacml($object['POLICY']->content);
      $xacml->managementRule->addUser(saskdora_get_primary_investigator($object));
      $xacml->managementRule->addUser($add);
      $xacml->managementRule->removeUser($remove);
      $object['POLICY']->content = $xacml->getXmlString();
    } catch (Exception $e) {
      $msg = t('Could not update the POLICY datastream on Object %pid .', array('%pid' => $object->id));
      throw new Exception($msg, $e->getCode(), $e);
    }
  }
}