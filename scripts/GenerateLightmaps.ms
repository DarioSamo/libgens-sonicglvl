/*
	Script heavily modified from http://www.proun-game.com/. Originally written by Joost "Oogst" van Dongen.
*/

renderPath = undefined

--This function checks whether a new unwrap should be created for channel 2 and does so if necessary.
--	Param obj: the object to work on.
--	Param createUnwrap: whether to add a new unwrap modifier if an unwrap existed already.
--	Param collapseAfterUnwrap: whether to collapse the modifier-stack after adding the unwrap modifier.
function handleUnwrapping obj createUnwrap collapseAfterUnwrap =
(
	--Check whether an unwrap in channel 2 already exists.
	addModifier obj (edit_mesh())
	local channel2UnwrapExists = meshop.getMapSupport obj 2
	deleteModifier obj 1

	--Create a new unwrap in channel 2 if requested or if it does not exist yet.
	if createUnwrap OR NOT channel2UnwrapExists then
	(
		--Add the unwrap-modifier.
		addModifier obj (unwrap_UVW())

		--The unwrap is put in channel 2, so as not to disturb the unwrap for the diffuse texture.
		obj.modifiers[1].setMapChannel 2

		--This is a bit of a hack, used to force 3dsmax to update the modifier dialogue. This is
		--necessary for flattenMap to work.
		classOf obj
		
		--Select all textureTriangles, to apply the flattenMap on.
		selectionBitArray = #{}
		for i = 1 to obj.modifiers[1].numberPolygons() do
		(
			append selectionBitArray i
		)
		obj.modifiers[1].selectPolygons selectionBitArray

		--Generate an automatic unwrap.
		obj.modifiers[1].flattenMap 50 #([1,0,0], [-1,0,0], [0,1,0], [0,-1,0], [0,0,1], [0,0,-1]) 0.01 true 0 true true

		--Collapse the modifier stack (including the unwrap modifier) if requested.
		if collapseAfterUnwrap then
		(
			collapseStack obj
		)
	)
)

function resetMap bakeobject map resolutionInPixels applyColorMapping =
(
	map.autoSzOn = false
	map.enabled = true
	map.filenameUnique = true
	map.filterOn = true
	map.outputSzX = resolutionInPixels
	map.outputSzY = resolutionInPixels
	map.targetMapSlotName = ""
	map.fileName = filenameFromPath map.fileType
	
	local param_index = bakeobject.findParam map "Apply color mapping"
	if (param_index != 0) then (
		bakeobject.setParamValue map param_index applyColorMapping
	)
)

--The function that actually performs the generation of the lightmaps.
--	Param createUnwrap: whether to add a new unwrap modifier.
--	Param collapseAfterUnwrap: whether to collapse the modifier-stack after adding the unwrap modifier.
--	Param highestResolutionPower: the highest resolution lightmap a large object can get.
function generateLightMaps resolutionMode createUnwrap collapseAfterUnwrap highestResolutionPower resolutionMultiplier skipExisting =
(
	max modify mode
	
	renderFileNames = #()
	renderFileSizes = #()
	renderFileSize = 0
	
	if (resolutionMode == 1) then (
		renderfilePath = getOpenFileName caption:"Select the render file to read from" types:"Render List (*.txt)|*.txt"
		if (renderfilePath == undefined) then return 0
		
		renderFile = openFile renderfilePath
		if renderFile != undefined then (
			while not eof renderFile do (
				renderFileSize += 1
				renderLine = readLine renderFile
				renderTokens = filterString renderLine " " splitEmptyTokens:true
				renderFileSizes[renderFileSize] = renderTokens[1] as integer
				renderFileNames[renderFileSize] = renderTokens[2] as string
			)
			
			close renderFile
		)
	)
	
	renderPath = getSavePath caption:"Select the folder to save to"
	if (renderPath == undefined) then return 0
	
	--These are the objects to be processed.
	local selectedObjects = copy ($selection as array) #noMap
	
	--Process every object.
	for obj in selectedObjects do
	(
		--abort operation if escape is pressed
		if keyboard.escPressed then
		(
			exit
		)

		--Only generate a lightmap if the object is geometry.
		if isKindOf obj GeometryClass AND NOT isKindOf obj TargetObject then
		(
			--Select the object.
			select obj
			
			local lightmapFilePath = renderPath + "\\" + obj.name + "_lightmap.png"
			local shadowmapFilePath = renderPath + "\\" + obj.name + "_shadowmap.png"
			local renderFiles = true
			
			if (skipExisting) then (
				local lightmapExists = doesFileExist lightmapFilePath
				local shadowmapExists = doesFileExist shadowmapFilePath
				if (lightmapExists AND shadowmapExists) then (
					renderFiles = false
				)
			)
			
			if (renderFiles) then (
				--Create an unwrap in channel 2 if requested or necessary.
				handleUnwrapping obj createUnwrap collapseAfterUnwrap
				
				--Calculate the resolution in pixels to be used for this object.
				local resolutionInPixels = pow 2 highestResolutionPower
				for RenderItem = 1 to renderFileSize do (
					if (obj.name == renderFileNames[RenderItem]) then (
						resolutionInPixels = renderFileSizes[RenderItem]
						exit
					)
				)
				
				if (resolutionMode == 1) then (
					resolutionInPixels *= pow 2 (resolutionMultiplier - 1)
				)

				--Settings for the baking of this object.
				local bakeobject = obj.INodeBakeProperties
				local lmap = VRayRawGlobalIlluminationMap()
				local smap = VRayMatteShadowMap()
				--local cmap = VRayCompleteMap()

				lmap.elementName = "bakedLightmap"
				lmap.filetype = lightmapFilePath
				resetMap bakeobject lmap resolutionInPixels 1
			
				smap.elementName = "bakedShadowmap"
				smap.filetype = shadowmapFilePath
				resetMap bakeobject smap resolutionInPixels 0
				
				--cmap.elementName = "bakedCompletemap"
				--cmap.filetype = renderPath + "\\" + obj.name + "_completemap.png"
				--resetMap bakeobject cmap resolutionInPixels 0

				--Attach the baking to this object.
				bakeobject.removeAllBakeElements()
				bakeobject.addBakeElement lmap
				bakeobject.addBakeElement smap
				--bakeobject.addBakeElement cmap
				bakeobject.nDilations = 4
				bakeObject.bakeEnabled = true
				bakeObject.bakeChannel = 2

				--let the currently selected object be baked.
				vfb = render rendertype:#BakeSelected outputwidth:resolutionInPixels outputheight:resolutionInPixels vfb:true
				close vfb
				
				--Baking is finished for this object, so reset the baking settings for this object.
				bakeobject.removeAllBakeElements()
			)
		)
	)
	
	--Recover the selection the user made before calling this script.
	select selectedObjects
)


--The user interface for the lightmap generator.
rollout lightmapGenerator "Lightmap Generator"
(
	Group "Resolution"
	(
		RadioButtons resolutionModeRadioButtons "Resolution mode:" labels:#("Render file", "Manual") default:2
		RadioButtons resolutionRadioButtons "Resolution:" labels:#("32", "64", "128", "256", "512", "1024", "2048", "4096") default:5
		RadioButtons resolutionMultiplierButtons "Multiplier:" labels:#("1", "2", "4") default:1 enabled:false
	)
	
	Group "Channel 2 unwrap"
	(
		CheckBox unwrapCheckBox "Create new unwrap" checked:false
		CheckBox collapseCheckBox "Collapse modifiers" checked:false enabled:false
	)
	
	Group "Files"
	(
		CheckBox skipExistingCheckBox "Skip existing files" checked:false
	)
	
	Button executeButton "Start lightmap generation"
	
	on unwrapCheckBox changed newState do
	(
		collapseCheckBox.enabled = newState
	)
	
	on resolutionModeRadioButtons changed newState do
	(
		resolutionMultiplierButtons.enabled = newState == 1
	)
	
	on executeButton pressed do
	(
		generateLightMaps resolutionModeRadioButtons.state \
						  unwrapCheckBox.checked \
						  collapseCheckBox.checked \
						  (resolutionRadioButtons.state + 4) \
						  resolutionMultiplierButtons.state \
						  skipExistingCheckBox.state
	)
)

--Put the user interface on the screen.
createDialog lightmapGenerator