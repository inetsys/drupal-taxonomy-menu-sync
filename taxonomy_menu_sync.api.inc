<?php

/**
 * @param object $new_term
 * @param object $old_term
 */
function hook_taxonomy_term_sync_modified($new_term, $old_term){
	$query = new EntityFieldQuery();
	$query->entityCondition('entity_type', 'node');
	$query->fieldCondition('FIELD_NAME', 'tid', $old_term->tid);
	$result = $query->execute();

	if(isset($result['node'])){
		$nodes = entity_load('node', array_keys($result['node']));

		foreach($nodes as $node){
			// Update each node with new taxonomy term tid
			// ...
		}
	}
}