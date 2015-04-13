class CreateLoc < ActiveRecord::Migration
  def change
  	create_table :bag_locations do |t|
  		t.decimal :latitude
  		t.decimal :longitude
  	end	
  end
end