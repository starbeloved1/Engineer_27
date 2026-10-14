from pathlib import Path
import sys,json,collections
import numpy as np
import xml.etree.ElementTree as E
import fcl
sys.path.insert(0,str(Path.cwd()/'scripts'))
import validate_collision_meshes as validation
import merge_link_meshes as merger
assets=Path('assets').resolve()
source=assets/'source/arm.urdf'
root=E.parse(source).getroot()
links={link.get('name'):link for link in root.findall('link')}
names=['link_1','link_4','link_5']
objects={name:validation.load_object(assets/'source/merged_v1/meshes/visual'/f'{name}.stl') for name in names}
instances={}
for name in names:
 offset=0; rows=[]
 for i,spec in enumerate(merger.visual_specs(links[name],source)):
  path,count,*_=spec
  rows.append((offset,offset+count,path.name,i))
  offset+=count
 instances[name]=rows

def instance(name,index):
 for start,end,part,i in instances[name]:
  if start<=index<end:return {'part':part,'visual_instance':i,'source_triangle':int(index-start)}
 return {'unmapped_triangle':int(index)}

sweep=[]
for angle in [-.2,-.1,-.05,0,.005,.01,.02,.03,.04,.05,.075,.1,.15,.2]:
 T=validation.transforms(root,{'link_3_joint':angle})
 for name,obj in objects.items(): obj.setTransform(fcl.Transform(T[name][:3,:3],T[name][:3,3]))
 entry={'angle_rad':angle,'pairs':{}}
 for b in ('link_4','link_5'):
  result=fcl.CollisionResult()
  hit=fcl.collide(objects['link_1'],objects[b],fcl.CollisionRequest(num_max_contacts=64,enable_contact=True),result)
  entry['pairs'][b]={'collision':bool(hit)}
  if abs(angle-.1)<1e-9:
   contacts=[]
   for c in result.contacts:
    contacts.append({'link_1':instance('link_1',c.b1),'other':instance(b,c.b2),
                     'position_base_m':c.pos.tolist(),'reported_depth_m':c.penetration_depth})
   entry['pairs'][b]['contacts']=contacts
  if angle==0:
   result=fcl.DistanceResult()
   distance=fcl.distance(objects['link_1'],objects[b],fcl.DistanceRequest(enable_nearest_points=True),result)
   entry['pairs'][b]['distance_m']=distance
   entry['pairs'][b]['nearest_points_base_m']=[p.tolist() for p in result.nearest_points]
 sweep.append(entry)
print(json.dumps(sweep,ensure_ascii=False,indent=2))
Path('.cache/mesh_validation/interference_diagnosis.json').write_text(json.dumps(sweep,ensure_ascii=False,indent=2)+'\n')
