<?php

/**
 * @file
 * Defines batch functions for manipulating an objects collaborators.
 */

/**
 * Gets a batch definition which will set the given objects collaborators.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param array $collaborators
 *   The collaborators to set the object to.
 * @param bool $display_message
 *   TRUE to show the results of the batch as a message FALSE otherwise.
 *
 * @return array
 *   The batch definition.
 */
function saskdora_set_collaborators_batch_definition(AbstractObject $object, array $collaborators, $display_message = TRUE) {
  return array(
    'operations' => array(
      array(
        'saskdora_collaborators_start_batch_operation',
        array($object->id, array('display_message' => $display_message)),
      ),
      array(
        'saskdora_collaborators_set_collaborators_batch_operation',
        array($collaborators),
      ),
    ),
    'finished' => 'saskdora_collaborators_batch_finished',
    'title' => t('Granting access to @label ...', array('@label' => $object->label)),
    'init_message' => t('Preparing to grant access to @label ...', array('@label' => $object->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'saskdora') . '/includes/collaborators.batch.inc',
  );
}

/**
 * Batch definition which will recursively update an object's collaborators.
 *
 * The batch is non-destructive in that the child objects will not lose any
 * collaborators in which are not referenced in $remove.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param array $add
 *   The collaborators to add.
 * @param array $remove
 *   The collaborators to remove.
 * @param bool $display_message
 *   TRUE to show the results of the batch as a message FALSE otherwise.
 *
 * @return array
 *   The batch definition.
 */
function saskdora_update_collaborators_recursive_batch_definition(AbstractObject $object, array $add, array $remove, $display_message = TRUE) {
  return array(
    'operations' => array(
      array(
        'saskdora_collaborators_start_batch_operation',
        array($object->id, array('display_message' => $display_message)),
      ),
      array(
        'saskdora_find_descendants_batch_operation',
        array(),
      ),
      array(
        'saskdora_collaborators_update_collaborators_batch_operation',
        array($add, $remove),
      ),
    ),
    'finished' => 'saskdora_collaborators_batch_finished',
    'title' => t('Granting access to @label ...', array('@label' => $object->label)),
    'init_message' => t('Preparing to grant access to @label ...', array('@label' => $object->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'saskdora') . '/includes/collaborators.batch.inc',
  );
}

/**
 * Batch definition which will recursively set an object's collaborators.
 *
 * The batch is destructive in that the child objects will loose any
 * collaborators which are not referenced in $collaborators.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param array $collaborators
 *   The collaborators to grant access to the object.
 * @param bool $display_message
 *   TRUE to show the results of the batch as a message FALSE otherwise.
 *
 * @return array
 *   The batch definition.
 */
function saskdora_set_collaborators_recursive_batch_definition(AbstractObject $object, array $collaborators, $display_message = TRUE) {
  return array(
    'operations' => array(
      array(
        'saskdora_collaborators_start_batch_operation',
        array($object->id, array('display_message' => $display_message)),
      ),
      array(
        'saskdora_find_descendants_batch_operation',
        array(),
      ),
      array(
        'saskdora_collaborators_set_collaborators_batch_operation',
        array($collaborators),
      ),
    ),
    'finished' => 'saskdora_collaborators_batch_finished',
    'title' => t('Granting access to @label ...', array('@label' => $object->label)),
    'init_message' => t('Preparing to grant access to @label ...', array('@label' => $object->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'saskdora') . '/includes/collaborators.batch.inc',
  );
}

/**
 * Batch definition which will recursively set an object's collaborators.
 *
 * The batch is destructive in that the child objects will loose any
 * collaborators which are not referenced in $collaborators.
 *
 * @param AbstractObject $object
 *   The object to update.
 * @param string[] $primary_investigator
 *   The primary investigator.
 * @param string[] $collaborators
 *   The collaborators to grant access to the object.
 * @param bool $display_message
 *   TRUE to show the results of the batch as a message FALSE otherwise.
 *
 * @return array
 *   The batch definition.
 */
function saskdora_set_primary_investigator_and_collaborators_recursive_batch_definition(AbstractObject $object, $primary_investigator, array $collaborators, $display_message = TRUE) {
  return array(
    'operations' => array(
      array(
        'saskdora_collaborators_start_batch_operation',
        array($object->id, array('display_message' => $display_message)),
      ),
      array(
        'saskdora_find_descendants_batch_operation',
        array(),
      ),
      array(
        'saskdora_set_primary_investigator_and_collaborators_batch_operation',
        array($primary_investigator, $collaborators),
      ),
    ),
    'finished' => 'saskdora_collaborators_batch_finished',
    'title' => t('Granting access to @label ...', array('@label' => $object->label)),
    'init_message' => t('Preparing to grant access to @label ...', array('@label' => $object->label)),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'saskdora') . '/includes/collaborators.batch.inc',
  );
}

/**
 * Sets up a batch by initializing the context.
 *
 * @param string $pid
 *   The PID of the object that is the focus of this batch.
 * @param array $options
 *   Configuration options for the batch.
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_collaborators_start_batch_operation($pid, array $options, array &$context) {
  $options += array('display_message' => TRUE);
  $context['results'] = array(
    'options' => $options,
    'queried' => array(),
    'objects' => array($pid),
    // 'successful' an array of boolean values indicating operation success.
    'successful' => array(),
  );
}

/**
 * Populates the 'objects' section of batch context.
 *
 * Uses 'queried' section of the context to stored the objects whose descendants
 * have been queried.
 *
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_find_descendants_batch_operation(array &$context) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $results = &$context['results'];
  $objects = &$results['objects'];
  $queried = &$results['queried'];
  $remaining = array_diff($objects, $queried);
  $pid = $queried[] = array_pop($remaining);
  $object = islandora_object_load($pid);
  if (!$object) {
    watchdog('saskdora', 'Object %pid not found. It and its descendants could not be updated.', array('%pid' => $pid), WATCHDOG_ERROR);
    $context['results']['successful'][$pid] = FALSE;
    $context['finished'] = count($remaining) / count($objects);
  }
  else {
    $descendants = saskdora_get_immediate_descendants($object);
    $objects = array_unique(array_merge($objects, $descendants));
    $context['message'] = t('Fetching descendants of @label (@pid)...', array(
      '@pid' => $object->id,
      '@label' => $object->label,
    ));
  }
  $context['finished'] = count($queried) / count($objects);
}

/**
 * Batch operation that updates the POLICY to allow access to the collaborators.
 *
 * This POLICY changes is non-destructive in that other permissions will not be
 * lost.
 *
 * @param string[] $add
 *   The collaborators to add to the POLICY.
 * @param string[] $remove
 *   The collaborators to remove from the POLICY.
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_collaborators_update_collaborators_batch_operation(array $add, array $remove, array &$context) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $results = &$context['results'];
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($results['objects']);
  }
  $index = &$context['sandbox']['progress'];
  $pid = $context['results']['objects'][$index];
  $object = islandora_object_load($pid);
  if (!$object) {
    watchdog('saskdora', 'Object %pid not found and could not be updated.', array('%pid' => $pid), WATCHDOG_ERROR);
    $results['successful'][$pid] = FALSE;
  }
  else {
    try {
      saskdora_update_collaborators($object, $add, $remove);
      watchdog('saskdora', 'Updated collaborators on %pid', array('%pid' => $pid));
      $results['successful'][$pid] = TRUE;
    }
    catch (Exception $e) {
      watchdog_exception('saskdora', $e);
      $results['successful'][$pid] = FALSE;
    }
  }
  ++$index;
  $context['finished'] = $index / $context['sandbox']['total'];
  $context['message'] = t('Updating @label (@pid)...', array(
    '@pid' => $object->id,
    '@label' => $object->label,
  ));
}

/**
 * Batch operation that updates the POLICY to allow access to the collaborators.
 *
 * This POLICY changes is destructive other permissions will be lost.
 *
 * @param string[] $collaborators
 *   The collaborators to set on the POLICY.
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_collaborators_set_collaborators_batch_operation(array $collaborators, array &$context) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $results = &$context['results'];
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($results['objects']);
  }
  $index = &$context['sandbox']['progress'];
  $pid = $context['results']['objects'][$index];
  $object = islandora_object_load($pid);
  if (!$object) {
    watchdog('saskdora', 'Object %pid not found and could not be updated.', array('%pid' => $pid), WATCHDOG_ERROR);
    $results['successful'][$pid] = FALSE;
    return;
  }
  else {
    try {
      saskdora_set_collaborators($object, $collaborators);
      watchdog('saskdora', 'Set collaborators on %pid', array('%pid' => $pid), WATCHDOG_NOTICE);
      $results['successful'][$pid] = TRUE;
    }
    catch (Exception $e) {
      watchdog_exception('saskdora', $e);
      $results['successful'][$pid] = FALSE;
    }
  }
  ++$index;
  $context['finished'] = $index / $context['sandbox']['total'];
  $context['message'] = t('Updating @label (@pid)...', array(
    '@pid' => $object->id,
    '@label' => $object->label,
  ));
}

/**
 * Batch operation that updates the POLICY to allow access to the collaborators.
 *
 * The Object owner will be set to the given primary investigator.
 *
 * This POLICY changes is destructive other permissions will be lost.
 *
 * @param string $primary_investigator
 *   The user name of the primary investigator.
 * @param string[] $collaborators
 *   The collaborators to set on the POLICY.
 * @param array $context
 *   The context of this batch operation.
 */
function saskdora_set_primary_investigator_and_collaborators_batch_operation($primary_investigator, array $collaborators, array &$context) {
  module_load_include('inc', 'saskdora', 'includes/utilities');
  $results = &$context['results'];
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($results['objects']);
  }
  $index = &$context['sandbox']['progress'];
  $pid = $context['results']['objects'][$index];
  $object = islandora_object_load($pid);
  if (!$object) {
    watchdog('saskdora', 'Object %pid not found and could not be updated.', array('%pid' => $pid), WATCHDOG_ERROR);
    $results['successful'][$pid] = FALSE;
    return;
  }
  else {
    try {
      $object->owner = $primary_investigator;
      saskdora_set_collaborators($object, $collaborators);
      watchdog('saskdora', 'Set collaborators on %pid', array('%pid' => $pid), WATCHDOG_NOTICE);
      $results['successful'][$pid] = TRUE;
    }
    catch (Exception $e) {
      watchdog_exception('saskdora', $e);
      $results['successful'][$pid] = FALSE;
    }
  }
  ++$index;
  $context['finished'] = $index / $context['sandbox']['total'];
  $context['message'] = t('Updating @label (@pid)...', array(
    '@pid' => $object->id,
    '@label' => $object->label,
  ));
}

/**
 * The batch 'finished' callback.
 */
function saskdora_collaborators_batch_finished($success, $results, $operations) {
  if ($success) {
    if ($results['options']['display_message']) {
      $total = count($results['successful']);
      $successful = count(array_filter($results['successful']));
      $failed = $total - $successful;
      $vars = array(
        '@successful' => $successful,
        '@failed' => $failed,
        '@total' => $total,
      );
      if ($successful) {
        drupal_set_message(t('Successfully updated @successful of @total objects', $vars), 'status');
      }
      if ($failed) {
        drupal_set_message(t('Failed to updated @failed of @total objects, Please please notify the administrator.', $vars), 'error');
      }
    }
  }
  else {
    // Generic Error Message.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}
