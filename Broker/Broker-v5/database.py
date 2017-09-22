import pymongo
from pymongo import MongoClient
from Data import *
import pprint
class mongodb():
	
	
	'''
	def __init__(self, db=None, collection=None, endponit=None):
	
		self.mongodb = db
        self.mongocollection = collection
        self.mongoendpoint = endpoint
	'''
	def connectDB(self, url, port):
		
		client = MongoClient(url, port)
		
		print "mongodatabase is launching"
		
		return client
	
	def getDB(self, client, db_name):
	
		db = client.db_name
		
		return db	
		
	def getCollection(self, client, collection):
		
		db = client.test_database
		coll = db.collection
		
		return coll
	
	def getObject(self, collection, key):
	
		array = collection.find_one(key)
		
		return array
	
	
	def updateDB(self, collection, key, post):
		
		coll = collection.update_one(key,
			{'$set' : post}
		)
		
		return collection
	
	
	def getContent(self, collection, key):
	
		array = collection.find_one(key)
		
		return array
	
	def retriveDB(self,client, db_name, collection):
		db = self.getDB(client, db_name)
		mycollection = self.getCollection(db, collection)
		return mycollection
	'''
	def retriveContent(self,client, db_name, collection,key):
		collection = self.retriveDB(client, db_name, collection)
		list = self.getContent(collection,key)
		return list
	'''
	def insertDB(self, collection, document):
	
		temp.id = collection.insert_one(document).inserted_id
	
	def retriveObject(self, client):
		db = client.test_database
		update = db.update
		list = update.find_one()
		return list
	
	def merge_two_dicts(self,x, y):
		"""Given two dicts, merge them into a new dict as a shallow copy."""
		z = x.copy()
		z.update(y)
		return z
	
	def initDB(self, client, key):
		
		db = client.test_database
		
		update = db.update
		update.delete_one(key)
		if (update.find_one(key) == None):
			print "new database"
			content = {
				Data.USER_INPUT:"Unset",
				Data.REQUEST_FLAG:"old",
				Data.NUM_CLIENTS:"Unset",
				Data.CLIENT_NUM:"Unset",
				Data.CLIENT_LIST:"Unset",
				Data.NUM_NODE2UPDATE:"0",
				Data.NUM_PROCESSED_NODE:"0",
				Data.NUM_SUCCESSFUL_NODE:"0",
				Data.NUM_FAILED_NODE:"0",
				Data.UPDATE_URL:"Unset",
				Data.TIME_CONSUMPTION:"",
				Data.update_time_in_broker:"0",
				Data.CLIENT_UPDATE_RESULT:""
			}	
			init_post = self.merge_two_dicts(key, content)
			update_id = update.insert_one(init_post).inserted_id
		
		list = pprint.pprint(update.find_one(key))
		
		print "the initial post of update collection is confirmed"
		
		print list
		
		return update
		
	def initClientDB(self, client, key):
		
		db = client.test_database
		
		update = db.update
		
		if (update.find_one(key) == None):
			print "new database"
			content = { 
				Data.USER_INPUT:"None",
				Data.UPDATE_URL:"Unset",
				Data.UPDATE_STATE:"idle",
				Data.UPDATE_RESULT:"0",
				Data.update_time_in_device:"0"
			}	
		
		
			init_post = self.merge_two_dicts(key, content)
			update_id = update.insert_one(init_post).inserted_id
		print "database is exsited"
		list = pprint.pprint(update.find_one(key))
		
		print "the initial post of update collection is confirmed"
		
		print list
		
		return update
		
	def mongodb_connection(self,db_ip):
		maxSevSelDelay = 3
		try:
			client = MongoClient(db_ip,27017,
                                     serverSelectionTimeoutMS=maxSevSelDelay)
			client.server_info() # force connection on a request as the
                         # connect=True parameter of MongoClient seems
                         # to be useless here 
		except pymongo.errors.ServerSelectionTimeoutError as err:
			# do whatever you need
			print(err)
			
			return False
			
		return True
		
	
		

		
		
		
