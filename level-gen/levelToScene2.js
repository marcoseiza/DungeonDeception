const fs = require('fs');

const args = process.argv.slice(2);
if (args.length < 2) {
    console.log("You need to provide two arguments: the input file name and the output file name.")
    process.exit(1)
}

const pixelWidth = 48, pixelHeight = 48;

const grassTextures = [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13];

const levelToScene2 = (fileName) => {
    let jsonData = fs.readFileSync(fileName, 'utf-8');
    let data = JSON.parse(jsonData);
    const { roomName, width, height, layout, decorations, enemies } = data;

    return {
        "comments": `This is the root node for the world scene for room ${roomName}.`,
        "type": "Node",
        "format": {
            "type": "Anchored"
        },
        "layout": {
            "x_anchor": "left",
            "y_anchor": "bottom"
        },
        "children": {
            "tiles": {
                "type": "Node",
                "format": {
                    "type": "Grid",
                    "width": width,
                    "height": height
                },
                "data": {
                    "anchor": [0, 0],
                    "size": [pixelWidth * width, pixelHeight * height],
                },
                "layout" : {
                    "x_anchor" : "bottom",
                    "y_anchor" : "left"
                },
                "children": Object.assign({}, ...layout.reduce((tile, type, index) => {
                    if (type == 0) return tile;
                    let row = Math.floor(index / width);
                    let col = index % width;
                    let obj = {};
                    obj[`tile-(${col}-${height - row - 1})`] = {
                        "type": "Node",
                        "format": {
                            "type": "Anchored"
                        },
                        "layout": {
                            "x_index": col,
                            "y_index": height - row - 1,
                            "x_anchor": "left",
                            "y_anchor": "bottom",
                        },
                        "children": {
                            "tile": {
                                "type": "Widget",
                                "data": {
                                    "key": `tile-${type}`,
                                    "variables": getVariables(type, row)
                                }
                            }
                        }
                    }

                    tile.push(obj);
                    return tile;
                }, []))
            },
            "decorations": {
                "type": "Node",
                "format": {
                    "type": "Grid",
                    "width": width,
                    "height": height
                },
                "data": {
                    "anchor": [0, 0],
                    "size": [pixelWidth * width, pixelHeight * height],
                },
                "layout" : {
                    "x_anchor" : "bottom",
                    "y_anchor" : "left"
                },
                "children": getDecorations(width, height, decorations, layout)
            },
            "enemies": {
                "type": "Node",
                "format": {
                    "type": "Grid",
                    "width": width,
                    "height": height
                },
                "data": {
                    "anchor": [0, 0],
                    "size": [pixelWidth * width, pixelHeight * height],
                },
                "layout" : {
                    "x_anchor" : "bottom",
                    "y_anchor" : "left"
                },
                "children": Object.assign({}, ...enemies.reduce((enemy, info, index) => {
                    let obj = {};
                    obj[`enemy-(${info["position"][0]}-${info["position"][1]})-${index}`] = {
                        "type": "Node",
                        "format": {
                            "type": "Anchored"
                        },
                        "data": {
                            "anchor": [0, 0],
                            "size": [pixelWidth, pixelHeight],
                        },
                        "layout": {
                            "x_index": Math.floor(info["position"][0]),
                            "y_index": Math.floor(info["position"][1]),
                            "x_anchor": "left",
                            "y_anchor": "bottom",
                        },
                        "children": {
                            "enemy": {
                                "type": info["type"],
                                "data": {
                                    "anchor": [0, 0],
                                    "size": [pixelWidth, pixelHeight],
                                },
                                "layout": {
                                    "x_anchor": "left",
                                    "y_anchor": "bottom",
                                    "x_offset": info["position"][0] - Math.floor(info["position"][0]),
                                    "y_offset": info["position"][1] - Math.floor(info["position"][1])
                                }
                            }
                        }
                    }
                    enemy.push(obj);
                    return enemy;
                }, []))
            }
        }
        
    }
}

const getDecorations = (width, height, decorations, layout) => {
    if (decorations.length > 0) {
        return Object.assign({}, ...decorations.reduce((decoration, info, index) => {
            let obj = {};

            let col = Math.floor(info["position"][0]);
            let row = Math.floor(info["position"][1]);
            
            let xOffset = info["position"][0] - col;
            let yOffset = info["position"][1] - row;

            obj[`decoration-(${info["position"][0]}-${info["position"][1]})-${index}`] = {
                "type": "Node",
                "format": {
                    "type": "Anchored"
                },
                "data": {
                    "size": [pixelWidth, pixelHeight]
                },
                "layout": {
                    "x_index": col,
                    "y_index": row,
                    "x_anchor": "left",
                    "y_anchor": "bottom",
                },
                "children": {
                    "decoration": {
                        "type": "Widget",
                        "data": {
                            "key": `tile-${info["type"]}`,
                            "variables": {
                                "priority": -0.95, // above the floor but below the shadows
                                "x_offset_frac": xOffset,
                                "y_offset_frac": yOffset
                            }
                        }
                    }
                }
            }
            decoration.push(obj);
            return decoration;
        }, []))

    } else { // RANDOMLY GENERATE GRASS
        return Object.assign({}, ...layout.reduce((decoration, type, index) => {
            if (type != 1) return decoration; // Only on grass tiles.
            if (Math.random() > 0.3) return decoration; // Only sometimes.

            let obj = {};
            let row = Math.floor(index / width);
            let col = index % width;

            let xOffset = Math.random() * 0.5;
            let yOffset = Math.random() * 0.5;

            obj[`decoration-(${col}-${height - row - 1})-${index}`] = {
                "type": "Node",
                "format": {
                    "type": "Anchored"
                },
                "data": {
                    "size": [pixelWidth, pixelHeight]
                },
                "layout": {
                    "x_index": col,
                    "y_index": row,
                    "x_anchor": "left",
                    "y_anchor": "bottom",
                },
                "children": {
                    "decoration": {
                        "type": "Widget",
                        "data": {
                            "key": `tile-${grassTextures[Math.floor(Math.random() * grassTextures.length)]}`,
                            "variables": {
                                "priority": -0.95, // above the floor but below the shadows
                                "x_offset_frac": xOffset,
                                "y_offset_frac": yOffset
                            }
                        }
                    }
                }
            }
            decoration.push(obj);
            return decoration;
        }, []))
    }

}

const getVariables = (type, row) => {
    // Will be able to enumerate all the specific variables for each tile type.
    // This includes putting decorations on tiles, or setting the floor type
    // for other tiles.
    switch (type) {
        case 1: // Floor
            return { };
        case 200: // Door horizontal
            return {
                "left-pillar-priority": row,
                "left-pillar-off-priority": row,
                "laser-priority": row,
                "right-pillar-priority": row,
                "right-pillar-off-priority": row,
                "positive-edge-1": row - 1,
                "positive-edge-2": row - 1,
                "positive-edge-3": row - 1,
                "positive-edge-4": row - 1,
                "positive-edge-5": row - 1,
                "negative-edge-1": row + 1,
                "negative-edge-2": row + 1,
                "negative-edge-3": row + 1,
                "negative-edge-4": row + 1,
                "negative-edge-5": row + 1,
            };
        case 201: // Door Vertical
            return {
                "top-pillar-priority": row - 1,
                "top-pillar-off-priority": row - 1,
                "bottom-pillar-priority": row + 1,
                "bottom-pillar-off-priority": row + 1,
                "positive-edge-1": row + 2,
                "positive-edge-2": row + 1,
                "positive-edge-3": row,
                "positive-edge-4": row - 1,
                "positive-edge-5": row - 2,
                "negative-edge-1": row + 2,
                "negative-edge-2": row + 1,
                "negative-edge-3": row,
                "negative-edge-4": row - 1,
                "negative-edge-5": row - 2,
            };
        default:
            return ({
                "priority": row
            });
    }
}

const saveToFile = (fileName, scene2Obj) => {
    const data = JSON.stringify(scene2Obj, null, 2);

    fs.writeFile(fileName, data, (err) => {
        if (err) { console.log(err); }
        console.log("JSON data saved.");
    });
}

const scene2Obj = levelToScene2(args[0]);
saveToFile(args[1], scene2Obj);
