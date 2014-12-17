<?php

/**
 * @param object $new_term
 * @param object $old_term
 */
function hook_taxonomy_term_sync_modified($new_term, $old_term){
	$query = db_select('taxonomy_index', 'ti');
	$query->join('node', 'n', 'ti.nid = n.nid');
	$nids = $query
		->fields('ti', array('nid'))
		->condition('ti.tid', $term->tid)
		->condition('n.type', 'NODE_TYPE')
		->execute()
		->fetchCol();
	$nodes = entity_load('node', $nids);

	foreach($nodes as $node){
		// Update each node with new taxonomy term tid
		// ...
	}
}

/**
 * @param object $term
 */
function sitetype_article_taxonomy_term_sync_deleted($term){
	$query = db_select('taxonomy_index', 'ti');
	$query->join('node', 'n', 'ti.nid = n.nid');
	$nids = $query
		->fields('ti', array('nid'))
		->condition('ti.tid', $term->tid)
		->condition('n.type', 'NODE_TYPE')
		->execute()
		->fetchCol();
	$nodes = entity_load('node', $nids);

	foreach($nodes as $node){
		// Update each node deleting old taxonomy term tid
		// ...
	}
}

/**
 * @param array $item
 * @param object $term
 * @param array $bundle
 * @return array
 */
function hook_menu_item_element_bundle_operations($item, $term, $bundle){
	return array(
		'add_page' => array(
			'#type' => 'link',
			'#title' => t('add new content'),
			'#href' => 'node/add/page',
			'#options' => array(
				'query' => array(
					'field_taxonomy_term' => $term->tid
				),
			),
			'weight' => 20,
		)
	);
}