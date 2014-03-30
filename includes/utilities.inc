<?php

/**
 * @file
 * This file contains helper functions used throughout this module.
 */

/**
 * Gets the primary investigator user of the given object.
 *
 * @param AbstractObject $object
 *   The object to get the primary investigator of.
 *
 * @return mixed
 *   A fully-loaded $user object upon successful user load or FALSE if user
 *   cannot be loaded. A primary investigator can be a non drupal user such as
 */
function saskdora_get_primary_investigator(AbstractObject $object) {
  $name = saskdora_get_primary_investigator_name($object);
  return user_load_by_name($name);
}

/**
 * Gets the primary investigators username from the given object.
 *
 * @param AbstractObject $object
 *   The object to get the primary investigator of.
 *
 * @return string
 *   The name of the primary investigator.
 */
function saskdora_get_primary_investigator_name(AbstractObject $object) {
  return $object->owner;
}

/**
 * Checks if the user is the primary investigator of the given object.
 *
 * @param AbstractObject $object
 *   The object to check against.
 * @param object $account
 *   (optional) The account to check, if not given use currently logged in user.
 *
 * @return bool
 *   TRUE if the user is the primary investigator, FALSE otherwise.
 */
function saskdora_is_primary_investigator(AbstractObject $object, $account = NULL) {
  global $user;
  $account = isset($account) ? $account : $user;
  return saskdora_get_primary_investigator_name($object) == $account->name;
}

/**
 * Checks if the user is has the primary investigator role.
 *
 * @param object $account
 *   (optional) The account to check, if not given use currently logged in user.
 *
 * @return bool
 *   TRUE if the user is a primary investigator, FALSE otherwise.
 */
function saskdora_is_user_a_primary_investigator($account = NULL) {
  global $user;
  $account = isset($account) ? $account : $user;
  return in_array('primary investigator', $account->roles);
}

/**
 * Checks if the user is a collaborator of the given object.
 *
 * @param AbstractObject $object
 *   The object to check against.
 * @param object $account
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
 * @param bool $drupal_users_only
 *   Only get users which are valid Drupal users, ignore users like fedoraAdmin.
 * @param bool $exclude_primary_investigator
 *   Do not include the primary investigator in the list of collaborators.
 *
 * @return string[]
 *   The names of the collaborators.
 */
function saskdora_get_collaborators(AbstractObject $object, $drupal_users_only = FALSE, $exclude_primary_investigator = FALSE) {
  $collaborators = array();
  if (isset($object['POLICY'])) {
    $xacml = new Xacml($object['POLICY']->content);
    if ($xacml->managementRule->isPopulated()) {
      $collaborators = $xacml->managementRule->getUsers();
    }
  }
  if ($drupal_users_only) {
    foreach ($collaborators as $key => $collaborator) {
      if (user_load_by_name($collaborator) === FALSE) {
        unset($collaborators[$key]);
      }
    }
  }
  if ($exclude_primary_investigator) {
    $primary_investigator_name = saskdora_get_primary_investigator_name($object);
    foreach ($collaborators as $key => $collaborator) {
      if ($collaborator == $primary_investigator_name) {
        unset($collaborators[$key]);
      }
    }
  }
  return $collaborators;
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
  // Always allow fedoraAdmin and the primary user management access.
  $users = $collaborators;
  $users[] = 'fedoraAdmin';
  $users[] = saskdora_get_primary_investigator_name($object);
  $xacml->viewingRule->addUser($users);
  $xacml->managementRule->addUser($users);
  if (isset($object['POLICY'])) {
    try {
      $object['POLICY']->content = $xacml->getXmlString();
    }
    catch (Exception $e) {
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
    }
    catch (Exception $e) {
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
      // Always allow fedoraAdmin and the primary user management access.
      $add[] = 'fedoraAdmin';
      $add[] = saskdora_get_primary_investigator_name($object);
      $xacml->managementRule->addUser($add);
      $xacml->viewingRule->addUser($add);
      $xacml->managementRule->removeUser($remove);
      $xacml->viewingRule->removeUser($remove);
      $object['POLICY']->content = $xacml->getXmlString();
    }
    catch (Exception $e) {
      $msg = t('Could not update the POLICY datastream on Object %pid .', array('%pid' => $object->id));
      throw new Exception($msg, $e->getCode(), $e);
    }
  }
}

/**
 * Gets the objects decedent's in which the XACML Policy could be applied to.
 *
 * It's possible for this function to time out, if this becomes a problem we
 * can break it out across several batch operations.
 *
 * @param AbstractObject $object
 *   The object whose decedents will be returned.
 *
 * @return string[]
 *   The PIDs of all decedents of the given object.
 */
function saskdora_get_decedents(AbstractObject $object) {
  module_load_include('inc', 'islandora_xacml_editor', 'includes/batch');
  module_load_include('inc', 'islandora', 'includes/utilities');
  $query_choices = array();
  foreach (islandora_build_hook_list('islandora_xacml_editor_child_query', $object->models) as $hook) {
    $temp = module_invoke_all($hook, $object);
    if (!empty($temp)) {
      $query_choices = array_merge_recursive($query_choices, $temp);
    }
  }
  if (isset($query_choices['saskdora'])) {
    $query = new IslandoraXacmlEditorQuery($object->id, $query_choices['saskdora']);
    return $query->getPids();
  }
  return array();
}

/**
 * Gets the owner's name of the parent of the given object.
 *
 * Uses the fedora ontology to define the parent, if the parent is not defined
 * by the fedora ontology then this function will fail.
 *
 * @param AbstractObject $object
 *   The object whose parent owner will be returned.
 *
 * @return string
 *   The name of the parent object owner if found, FALSE otherwise.
 */
function saskdora_get_parent_owner_name(AbstractObject $object) {
  $query = <<<EOQ
SELECT ?object ?owner FROM <#ri> WHERE {
  {
    <info:fedora/{$object->id}> <fedora-rels-ext:isPartOf> ?object .
  }
  UNION {
    <info:fedora/{$object->id}> <fedora-rels-ext:isConstituentOf> ?object .
  }
  UNION {
    <info:fedora/{$object->id}> <fedora-rels-ext:isMemberOf> ?object .
  }
  UNION {
    <info:fedora/{$object->id}> <fedora-rels-ext:isSubsetOf> ?object .
  }
  UNION {
    <info:fedora/{$object->id}> <fedora-rels-ext:isMemberOfCollection> ?object .
  }
  UNION {
    <info:fedora/{$object->id}> <fedora-rels-ext:isDescriptionOf> ?object .
  }
  UNION {
    <info:fedora/{$object->id}> <fedora-rels-ext:isMetadataFor> ?object .
  }
  UNION {
    <info:fedora/{$object->id}> <fedora-rels-ext:isAnnotationOf> ?object .
  }
  ?object <fedora-model:state> <info:fedora/fedora-system:def/model#Active> ;
          <fedora-model:ownerId> ?owner .
}
LIMIT 1
EOQ;
  // The Primary Investigator, owns all the objects in their sub-graph.
  // $object->owner = $parent->owner;
  $results = $object->repository->ri->sparqlQuery($query, 1);
  if (isset($results[0]['owner']['value'])) {
    return $results[0]['owner']['value'];
  }
  return FALSE;
}

/**
 * Gets the owner of the parent of the given object.
 *
 * @param AbstractObject $object
 *   The object whose parent owner will be returned.
 *
 * @return object
 *   The owner of the parent object if found, FALSE otherwise.
 */
function saskdora_get_parent_owner(AbstractObject $object) {
  $owner_name = saskdora_get_parent_owner_name($object);
  return $owner_name ? user_load_by_name($owner_name) : FALSE;
}
