require 'net/http'
require 'net/https'
require 'uri'
require 'rubygems'
require 'sinatra'
require 'json/ext' # required for .to_json
require 'sinatra/activerecord'

class BagLocation < ActiveRecord::Base
end

post '/locations' do
	request.body.rewind
  	@bag_location_json = JSON.parse request.body.read
  	lat = @bag_location_json['latitude']
	long = @bag_location_json['longitude']
	bagLocation = BagLocation.create(latitude: lat, longitude: long)	
end

get '/locations' do
	locations = BagLocation.all.to_json
end